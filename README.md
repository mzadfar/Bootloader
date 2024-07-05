# Bare Metal Programming

The purpose of this repository is to start from practicing a simple and basic bootloader to a signed AES-128 one using Hardware Abstraction Layer (HAL) of STM32F439ZI developed by ST.

## Description

The following topics will be practised here on a NUCLEO-F439ZI board:
* A simple and basic bootloader
* Adding a ring buffer to bootloader
* Adding a state machine to bootloader for communication between host and flash
* Adding a flash earaser to bootloader
* Adding a timeout mechanism to bootloader
* Adding a syncing mechanism between host and bootloader
* Adding a uploasding firmware mechanism to  bootloader
* Adding a chaecking firmware mechanism to  bootloader
* Adding a AES-128 signing firmware mechanism to  bootloader

## Getting Started

### Dependencies

* Host can be any OS like Fedora 40 Linux in this case or Ubuntu, Windows etc.  
* arm-none-eabi-gcc should be installed.
* gcc should be installed.
* cmake should be installed.
* Download and install [ST-LINK](https://www.st.com/en/development-tools/stsw-link007.html) latest firmware.
* In this Fedora 40 OS, Download [ST-LINK](https://koji.fedoraproject.org/koji/buildinfo?buildID=2389744) v1/v2 JTAG/SWD debugging/flashing tool for STM32 microcontrollers
* After above installation, Cmake still has issue with LIBUSB, and in this Fedora 40 OS using "yum provides */libusb.h"
* LIBUSB should be installed.
* STM32F4 Standard Peripheral Library Expansion [STSW-STM32120](https://www.st.com/en/embedded-software/stm32-standard-peripheral-library-expansion/products.html) should be downloaded. Find the one that has "system" under "MCU Peripheral" column.

### Installing

* Check arm-none-eabi-gcc --version on a terminal. If it has not been installed, Fedora will suggest to install, and go with it.
* Check cmake --version on a terminal. If it has not been installed, Fedora will suggest to install, and go with it. Typically, it asks for gcc installation during installing cmake if you have not installed gcc yet.
* Install [ST-LINK](https://www.st.com/en/development-tools/stsw-link007.html) latest firmware using: sudo rpm -Uhv st-stlink-udev-rules-1.0.3-2-linux-noarch.rpm
* In this Fedora 40 OS, install [ST-LINK](https://koji.fedoraproject.org/koji/buildinfo?buildID=2389744) v1/v2 JTAG/SWD debugging/flashing tool for STM32 microcontrollers using: sudo rpm -Uhv stlink-1.7.0-7.fc40.x86_64.rpm
* cmake has issue with LIBUSB, and in this Fedora 40 OS using "yum provides */libusb.h" command find a version of LIBUSB that suit you, and install it using (in this case): sudo dnf install libusb1-devel-1.0.27-2.fc40.x86_64
* Download stlink-testing.zip to open a terminal inside the extracted folder and run cmake.
* Run make

### Executing program

#### A simple and basic bootloader
* First copy folder"[Libraries](https://github.com/ahasanzadeh/BareMetal/tree/main/008_VirtualComTxCharacter)" into the \Shared folder inside this prject folder.

#### Adding a ring buffer to bootloader
* First copy folder"[Libraries](https://github.com/ahasanzadeh/BareMetal/tree/main/008_VirtualComTxCharacter)" into the \Shared folder inside this prject folder.

#### Adding a state machine to bootloader for communication between host and flash
* First copy folder"[Libraries](https://github.com/ahasanzadeh/BareMetal/tree/main/008_VirtualComTxCharacter)" into the \Shared folder inside this prject folder.

#### Adding a flash earaser to bootloader
* First copy folder"[Libraries](https://github.com/ahasanzadeh/BareMetal/tree/main/008_VirtualComTxCharacter)" into the \Shared folder inside this prject folder.

#### Adding a timeout mechanism to bootloader
* First copy folder"[Libraries](https://github.com/ahasanzadeh/BareMetal/tree/main/008_VirtualComTxCharacter)" into the \Shared folder inside this prject folder.

#### Adding a syncing mechanism between host and bootloader
* First copy folder"[Libraries](https://github.com/ahasanzadeh/BareMetal/tree/main/008_VirtualComTxCharacter)" into the \Shared folder inside this prject folder.

#### Adding a uploasding firmware mechanism to  bootloader
* First copy folder"[Libraries](https://github.com/ahasanzadeh/BareMetal/tree/main/008_VirtualComTxCharacter)" into the \Shared folder inside this prject folder.

#### Adding a chaecking firmware mechanism to  bootloader
* First copy folder"[Libraries](https://github.com/ahasanzadeh/BareMetal/tree/main/008_VirtualComTxCharacter)" into the \Shared folder inside this prject folder.

#### Adding a AES-128 signing firmware mechanism to  bootloader
* First copy folder"[Libraries](https://github.com/ahasanzadeh/BareMetal/tree/main/008_VirtualComTxCharacter)" into the \Shared folder inside this prject folder.

## Help

TBD

## Authors

Contributors names and contact info

TBD

## Version History

* 0.1
    * Initial Release

## License

This project is licensed under the FREE License - see the LICENSE.md file for details

## Acknowledgments

Inspiration, code snippets, etc.
* [ah](https://github.com/ahasanzadeh/)