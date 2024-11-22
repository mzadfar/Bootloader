#python FirmwareUploader.py signed.bin 

import serial
import time
import sys
import os
import struct
import pathlib

# Constants for the packet protocol
PACKET_LENGTH_BYTES = 1
PACKET_DATA_BYTES = 16
PACKET_CRC_BYTES = 1
PACKET_CRC_INDEX = PACKET_LENGTH_BYTES + PACKET_DATA_BYTES
PACKET_LENGTH = PACKET_LENGTH_BYTES + PACKET_DATA_BYTES + PACKET_CRC_BYTES

PACKET_ACKNOWLEDGE_DATA0 = 0x15
PACKET_RETRANSMIT_DATA0 = 0x19

BOOTLOADER_PACKET_SYNC_OBSERVED_DATA0 = 0x20
BOOTLOADER_PACKET_FW_UPDATE_REQUEST_DATA0 = 0x31
BOOTLOADER_PACKET_FW_UPDATE_RESPONSE_DATA0 = 0x37
BOOTLOADER_PACKET_DEVICE_ID_REQUEST_DATA0 = 0x3C
BOOTLOADER_PACKET_DEVICE_ID_RESPONSE_DATA0 = 0x3F
BOOTLOADER_PACKET_FW_LENGTH_REQUEST_DATA0 = 0x42
BOOTLOADER_PACKET_FW_LENGTH_RESPONSE_DATA0 = 0x45
BOOTLOADER_PACKET_FW_READY_FOR_DATA_DATA0 = 0x48
BOOTLOADER_PACKET_UPDATE_SUCCESSFUL_DATA0 = 0x54
BOOTLOADER_PACKET_NOT_ACKNOWLEDGE_DATA0 = 0x59

FW_INFO_SENTINEL = (0xDEADC0DE)
BOOTLOADER_SIZE = (0x10000)
FLASH_BASE = 0x08000000
MAIN_APP_START = (FLASH_BASE + BOOTLOADER_SIZE)
VECTOR_TABLE_SIZE = 0x1B0 #sizeof(g_pfnVectors[])
#FIRMWARE_INFO_SIZE = (10 * 4) #sizeof(fwInfo_t)

#FW_INFO_ADDRESS = (MAIN_APP_START + VECTOR_TABLE_SIZE)
#FW_INFO_VALIDATE_ADDRESS = (VECTOR_TABLE_SIZE + FIRMWARE_INFO_SIZE)
#FW_INFO_SENTINEL_OFFSET = (VECTOR_TABLE_SIZE + (0 * 4))
FW_INFO_DEVICE_ID_OFFSET = (VECTOR_TABLE_SIZE + (1 * 4))
#FW_INFO_VERSION_OFFSET = (VECTOR_TABLE_SIZE + (2 * 4))
FW_INFO_LENGTH_OFFSET = (VECTOR_TABLE_SIZE + (3 * 4))
#FW_INFO_CRC32_OFFSET = (VECTOR_TABLE_SIZE + (9 * 4))
#FW_INFO_VALIDATE_LENGTH(fwLength) = (fwLength - sizeof(g_pfnVectors[]) + sizeof(fwInfo_t))


BOOTLOADER_SIZE = 0x10000

DEVICE_ID = 0x42
#SYNC_SEQ_0 = 0xC4;
#SYNC_SEQ_1 = 0x55;
#SYNC_SEQ_2 = 0x7E;
#SYNC_SEQ_3 = 0x10;
SYNC_SEQ = bytes([0xc4, 0x55, 0x7e, 0x10])
DEFAULT_TIMEOUT = 5000

# Details about the serial port connection
serial_path = "/dev/ttyACM0"
baud_rate = 115200 # set the correct port before run it

# CRC8 implementation
def crc8(data):
    crc = 0
    for byte in data:
        crc = (crc ^ byte) & 0xff
        for i in range(8):
            if (crc & 0x80):
                crc = ((crc << 1) ^ 0x07) & 0xff
            else:
                crc = (crc << 1) & 0xff
    return crc

def crc32(data, length):
    crc = 0xffffffff

    for i in range(length):
        byte = data[i]
        crc = (crc ^ byte) & 0xffffffff

        for j in range(8):
            mask = - (crc & 1) & 0xffffffff
            crc = ((crc >> 1) ^ (0xedb88320 & mask)) & 0xffffffff

    return (~crc) & 0xffffffff

#Test CRC8 works properly, CRC8 = 0x52
#data1 = [1, 25, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255]
#[9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]
#print("CRC :", crc8(data1))

# Async delay function, which gives the event loop time to process outside input
def delay(ms):
    return asyncio.sleep(ms / 1000)

class Logger:
    @staticmethod
    def info(message):
        print("[.] {}".format(message))

    @staticmethod
    def success(message):
        print("[$] {}".format(message))

    @staticmethod
    def error(message):
        print("[!] {}".format(message))

# Class for serialising and deserialising packets
class Packet:
    def __init__(self, length, data, crc):
        self.length = length
        self.data = data
        self.crc = crc

    def __init__(self, length, data, crc = None):
        self.length = length
        self.data = data

        bytesToPad = PACKET_DATA_BYTES - len(self.data)
        padding = bytes([0xff] * bytesToPad)
        self.data += padding

        if crc is None:
            self.crc = self.computeCrc()
        else:
            self.crc = crc

    @staticmethod
    def retx():
        return Packet(1, bytes([PACKET_RETRANSMIT_DATA0])).toBuffer()

    @staticmethod
    def ack():
        return Packet(1, bytes([PACKET_ACKNOWLEDGE_DATA0])).toBuffer()

    def computeCrc(self):
        allData = [self.length] + list(self.data)
        return crc8(allData)

    def toBuffer(self):
        return bytes([self.length]) + self.data + bytes([self.crc])

    def isSingleBytePacket(self, byte):
        if self.length != 1:
            return False
        if self.data[0] != byte:
            return False
        for i in range(1, len(self.data)):
            if self.data[i] != 0xff:
                return False
        return True

    def isAck(self):
        return self.isSingleBytePacket(PACKET_ACKNOWLEDGE_DATA0)

    def isRetx(self):
        return self.isSingleBytePacket(PACKET_RETRANSMIT_DATA0)
    
    @staticmethod
    def create_single_byte_packet(byte: int):
        return Packet(1, bytes([byte]))
    
# Serial port instance
uart = serial.Serial(serial_path, baud_rate)

# Packet buffer
packets = Packet(1, bytes([]))

last_packet = Packet(1, bytes([0xff]))

def write_packet(packet):
    uart.write(packet)
    last_packet = packet

# Serial data buffer, with a splice-like function for consuming data
rx_buffer = Packet(1, bytes([]))

def consume_from_buffer(newdata, n):
    consumed = newdata[len(newdata)-n:]
    rx_buffer.data = newdata[:len(newdata)-n]
    return consumed

# This function fires whenever data is received over the serial port. The whole
# packet state machine runs here.
def uart_rxdata(size):
    rxdata = uart.read(size)
    #print(f'Received {len(rxdata)} bytes through uart')
    # Add the data to the packet
    rx_buffer.data += rxdata
    # Can we build a packet?
    while len(rx_buffer.data) >= PACKET_LENGTH:
        #print('Building a packet')
        raw = consume_from_buffer(rx_buffer.data, PACKET_LENGTH)
        packet = Packet(raw[0], raw[1:PACKET_DATA_BYTES+1], raw[PACKET_CRC_INDEX])
        #print(list(packet.toBuffer()))
        computed_crc = packet.computeCrc() 
        
        # Need retransmission?
        if packet.crc != computed_crc:
            print(f'CRC failed, computed 0x{computed_crc:x}, got 0x{packet.crc:x}')
            write_packet(Packet.retx())
        
        # Are we being asked to retransmit?
        if packet.isRetx():
            print('Retransmitting last packet')
            write_packet(last_packet.toBuffer())
        
        # If this is an ack, move on
        if packet.isAck():
            print('It was an ack, nothing to do')

                # If this is an ack, move on
        if packet.isSingleBytePacket(BOOTLOADER_PACKET_NOT_ACKNOWLEDGE_DATA0):
            Logger.error('Received NACK')
            sys.exit(1)
        
        # Otherwise write the packet in to the buffer, and send an ack
        #print('Storing packet and acking')
        #packets.append(packet)
        write_packet(Packet.ack())
        return packet

# Function to allow us to await a packet
def wait_for_packet(timeout=DEFAULT_TIMEOUT):
    #timeWaited = 0
    #while len(packets) < 1:
    #    await delay(1)
    #    timeWaited += 1

    #    if timeWaited >= timeout:
    #        raise Error('Timed out waiting for packet')
    
    if uart.is_open:
        while True:
            size = uart.inWaiting()
            if size:
                packets = uart_rxdata(size)
                #send_packet = Packet(4, bytes([5, 6, 7, 8]))
                #send_packet.crc += 1 #Put an increament error on CRC
                #write_packet(send_packet.toBuffer())
                #print(list(packets.toBuffer()))
                if (packets):
                    return packets
            else:
                #print('no data')
                time.sleep(.1)
    else:
        print('serialPort not open')

def waitForSingleBytePacket(byte, timeout=DEFAULT_TIMEOUT):
    timeWaited = 0

    while True:
        #await delay(1000)
        #time.sleep(1)
        timeWaited += 1000

        packets = wait_for_packet(timeout)
        if len(packets.data) > 0:
            if packets.isSingleBytePacket(byte):
                return
            Logger.error(f'Unexpected packet received. Expected single byte 0x{byte:02x}, Received 0x{packets.data[0]:02x}')
            sys.exit(1)

        if timeWaited >= timeout:
            Logger.error('Timed out waiting for the correct sequence packet')
            sys.exit(1)
    
#async def syncWithBootloader(timeout=DEFAULT_TIMEOUT):
def syncWithBootloader(timeout=DEFAULT_TIMEOUT):
    timeWaited = 0

    while True:
        write_packet(SYNC_SEQ)
        #await delay(1000)
        time.sleep(1)
        timeWaited += 1000

        packets = wait_for_packet(timeout)
        if len(packets.data) > 0:
            if packets.isSingleBytePacket(BOOTLOADER_PACKET_SYNC_OBSERVED_DATA0):
                return
            Logger.error('Wrong packet observed during sync sequence')
            sys.exit(1)

        if timeWaited >= timeout:
            Logger.error('Timed out waiting for sync sequence observed')
            sys.exit(1)

#async def main():
def main():
    if len(sys.argv) < 2:
        print("usage: FirmwareUploader.py <signed firmware>")
        sys.exit(1)

    Logger.info('Reading the firmware image...')
    fileName = sys.argv[1]  # Replace with the actual filename
    #firmware_path = os.path.join(os.getcwd(), fileName)
    #fwImage = pathlib.Path(firmware_path).read_bytes()
    with open(os.path.join(os.getcwd(), fileName), 'rb') as f:
        fwImage = f.read()#[BOOTLOADER_SIZE:]
    fwLength = len(fwImage)
    Logger.success(f'Read firmware image ({fwLength} bytes)')

    '''
    Logger.info('Injecting into firmware information section')
    fwImage = bytearray(fwImage)
    struct.pack_into('<I', fwImage, FW_INFO_LENGTH_OFFSET, fwLength)
    struct.pack_into('<I', fwImage, FW_INFO_VERSION_OFFSET, 0x00000001)

    
    crcValue = crc32(fwImage[FW_INFO_VALIDATE_ADDRESS:FW_INFO_VALIDATE_ADDRESS + (fwLength - (VECTOR_TABLE_SIZE + FIRMWARE_INFO_SIZE))], (fwLength - (VECTOR_TABLE_SIZE + FIRMWARE_INFO_SIZE)))
    Logger.info(f'Computed CRC value: 0x{crcValue:08X} for firmware length of 0x{(fwLength - (VECTOR_TABLE_SIZE + FIRMWARE_INFO_SIZE)):08X}')
    struct.pack_into('<I', fwImage, FW_INFO_CRC32_OFFSET, crcValue)
    if (fwLength != len(fwImage)):
        Logger.error(f'Unexpected packet received. Expected single byte 0x{fwLength:02x}, Received 0x{len(fwImage):02x}')
        sys.exit(1)
    else:
        Logger.success(f'Firmware info packed into firmware image with ({fwLength} bytes)')
'''

    Logger.info('Attempting to sync with the bootloader')
    #await syncWithBootloader()
    syncWithBootloader()
    Logger.success('Synced!')

    Logger.info('Requesting firmware update')
    fwUpdatePacket = Packet.create_single_byte_packet(BOOTLOADER_PACKET_FW_UPDATE_REQUEST_DATA0)
    write_packet(fwUpdatePacket.toBuffer())
    waitForSingleBytePacket(BOOTLOADER_PACKET_FW_UPDATE_RESPONSE_DATA0)
    Logger.success('Firmware update request accepted')

    Logger.info('Waiting for device ID request')
    waitForSingleBytePacket(BOOTLOADER_PACKET_DEVICE_ID_REQUEST_DATA0)
    Logger.success('Firmware device ID request accepted')

    deviceID = fwImage[FW_INFO_DEVICE_ID_OFFSET]
    deviceIDPacket = Packet(2, bytes([BOOTLOADER_PACKET_DEVICE_ID_RESPONSE_DATA0, deviceID]))
    write_packet(deviceIDPacket.toBuffer())
    Logger.info(f'Responding with device ID: 0x{deviceID:02x}')

    Logger.info('Waiting for firmware length request')
    waitForSingleBytePacket(BOOTLOADER_PACKET_FW_LENGTH_REQUEST_DATA0)
    Logger.success('Firmware length request sent')

    Logger.info('Responding with firmware length')
    fwLengthPacketBuffer = bytearray(5)
    fwLengthPacketBuffer[0] = BOOTLOADER_PACKET_FW_LENGTH_RESPONSE_DATA0
    fwLengthPacketBuffer[1:5] = fwLength.to_bytes(4, byteorder='little')
    fwLengthPacket = Packet(5, fwLengthPacketBuffer)
    write_packet(fwLengthPacket.toBuffer())
    Logger.success('Firmware length request accepted')
    
    Logger.info('Waiting for a few seconds for main application to be erased...');
    time.sleep(2)
    Logger.info('Waiting for a few seconds for main application to be erased...');
    time.sleep(2)
    Logger.info('Waiting for a few seconds for main application to be erased...');
    time.sleep(2)
    Logger.info('Waiting for ready for data packet received')
    #waitForSingleBytePacket(BOOTLOADER_PACKET_FW_READY_FOR_DATA_DATA0)
    #Logger.success('Flash erased and ready for data packet received')

    bytesWritten = 0
    while (bytesWritten < fwLength):
        waitForSingleBytePacket(BOOTLOADER_PACKET_FW_READY_FOR_DATA_DATA0)
        dataBytes = fwImage[bytesWritten:bytesWritten + PACKET_DATA_BYTES]  # PACKET_DATA_BYTES
        dataLength = len(dataBytes)
        dataPacket = Packet(dataLength - 1, dataBytes)
        write_packet(dataPacket.toBuffer())
        bytesWritten += dataLength

        Logger.info(f"Wrote {dataLength} bytes ({bytesWritten}/{fwLength})")

    waitForSingleBytePacket(BOOTLOADER_PACKET_UPDATE_SUCCESSFUL_DATA0)  # BOOTLOADER_PACKET_UPDATE_SUCCESSFUL_DATA0
    Logger.success("Firmware update complete!")

main()
uart.close()

'''
serialPort = serial.Serial(port=serialPath, baudrate=baudRateValue)
serialPort.timeout = 2  # set read timeout
print(serialPort)  # debug serial.
print(serialPort.is_open)  # True for opened
if serialPort.is_open:
    while True:
        size = serialPort.inWaiting()
        if size:
            data = serialPort.read(size)
            data = list(data)                                                                    
            print("length :", size, "data :", data)
            del data[size-1]
            print("CRC :", crc8(data))
        else:
            print('no data')
        time.sleep(1)
else:
    print('serialPort not open')
serialPort.close()  # close z1serial if z1serial is open.
'''