import serial
import time

# Constants for the packet protocol
PACKET_LENGTH_BYTES = 1
PACKET_DATA_BYTES = 16
PACKET_CRC_BYTES = 1
PACKET_CRC_INDEX = PACKET_LENGTH_BYTES + PACKET_DATA_BYTES
PACKET_LENGTH = PACKET_LENGTH_BYTES + PACKET_DATA_BYTES + PACKET_CRC_BYTES

PACKET_ACK_DATA0 = 0x15
PACKET_RETX_DATA0 = 0x19

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

#Test CRC8 works properly, CRC8 = 0x52
data1 = [1, 25, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255]
#[9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]
print("CRC :", crc8(data1))

# Async delay function, which gives the event loop time to process outside input
def delay(ms):
    return asyncio.sleep(ms / 1000)

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
        return Packet(1, bytes([PACKET_RETX_DATA0])).toBuffer()

    @staticmethod
    def ack():
        return Packet(1, bytes([PACKET_ACK_DATA0])).toBuffer()

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
        return self.isSingleBytePacket(PACKET_ACK_DATA0)

    def isRetx(self):
        return self.isSingleBytePacket(PACKET_RETX_DATA0)

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
    print(f'Received {len(rxdata)} bytes through uart')
    # Add the data to the packet
    rx_buffer.data += rxdata
    # Can we build a packet?
    if len(rx_buffer.data) >= PACKET_LENGTH:
        print('Building a packet')
        raw = consume_from_buffer(rx_buffer.data, PACKET_LENGTH)
        packet = Packet(raw[0], raw[1:PACKET_DATA_BYTES+1], raw[PACKET_CRC_INDEX])
        print(list(packet.toBuffer()))
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
        
        # Otherwise write the packet in to the buffer, and send an ack
        print('Storing packet and acking')
        #packets.append(packet)
        write_packet(Packet.ack())
        return packet

# Function to allow us to await a packet
def wait_for_packet():
    #while len(packets) < 1:
        #await delay(1)
    if uart.is_open:
        while True:
            size = uart.inWaiting()
            if size:
                packets = uart_rxdata(size)
                send_packet = Packet(4, bytes([5, 6, 7, 8]))
                send_packet.crc += 1 #Put an increament error on CRC
                write_packet(send_packet.toBuffer())
                #if (packets):
                #    return packets
            else:
                print('no data')
            time.sleep(1)
    else:
        print('serialPort not open')


# Do everything in an async function so we can have loops, awaits etc
def main():
    print('waiting for packet...')
    packets = wait_for_packet()
    #packet = await wait_for_packet()
    packets = packets.toBuffer()
    packetLength = packets[0]
    packetData = list(packets[1:len(packets)-1])
    packetCRC = packets[len(packets)-1]
    print('Number of received packets:', packetLength)
    print('Received packets:', packetData)
    print('Received CRC:', packetCRC)

main()

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
