# Bare Metal Programming

The purpose of this repository is to start from practicing a simple and basic bootloader to a signed AES-128 one using Hardware Abstraction Layer (HAL) of STM32F439ZI developed by ST.

## Description

The following topics will be practiced here on a NUCLEO-F439ZI board:
* A simple and basic bootloader
* Adding a ring buffer to bootloader
* Adding a state machine to bootloader for communication between host and flash
* Adding a flash eraser to bootloader
* Adding a timeout mechanism to bootloader
* Adding a syncing mechanism between host and bootloader
* Adding an uploading firmware mechanism to bootloader
* Adding a checking firmware mechanism to bootloader
* Adding an AES-128 signing firmware mechanism to bootloader

## Getting Started

### Dependencies

* Host can be any OS like Fedora 42 Linux in this case or Ubuntu, Windows etc.  
* arm-none-eabi-gcc should be installed.
* gcc should be installed.
* cmake should be installed.
* Download and install [ST-LINK](https://www.st.com/en/development-tools/stsw-link007.html) latest firmware.
* In this Fedora 40 OS, Download [ST-LINK](https://koji.fedoraproject.org/koji/buildinfo?buildID=2389744) v1/v2 JTAG/SWD debugging/flashing tool for STM32 microcontrollers
* After above installation, Cmake still has issue with LIBUSB, and in this Fedora 40 OS using "yum provides */libusb.h"
* LIBUSB should be installed.
* STM32F4 Standard Peripheral Library Expansion [STSW-STM32120](https://www.st.com/en/embedded-software/stm32-standard-peripheral-library-expansion/products.html) should be downloaded. Find the one that has "system" under "MCU Peripheral" column. Please copy "Libraries" folder next to "Bootloader" folder (the folder contains all projects in this repository).

### Installing

* Check arm-none-eabi-gcc --version on a terminal. If it has not been installed, Fedora will suggest to install, and go with it.
* Check cmake --version on a terminal. If it has not been installed, Fedora will suggest installing, and go with it. Typically, it asks for gcc installation during installing cmake if you have not installed gcc yet.
* Install [ST-LINK](https://www.st.com/en/development-tools/stsw-link007.html) latest firmware using: sudo rpm -Uhv st-stlink-udev-rules-1.0.3-2-linux-noarch.rpm
* In this Fedora 40 OS, install [ST-LINK](https://koji.fedoraproject.org/koji/buildinfo?buildID=2389744) v1/v2 JTAG/SWD debugging/flashing tool for STM32 microcontrollers using: sudo rpm -Uhv stlink-1.7.0-7.fc40.x86_64.rpm
* cmake has issue with LIBUSB, and in this Fedora 40 OS using "yum provides */libusb.h" command find a version of LIBUSB that suit you, and install it using (in this case): sudo dnf install libusb1-devel-1.0.27-2.fc40.x86_64
* Download stlink-testing.zip to open a terminal inside the extracted folder and run cmake.
* Run make

### Executing program

#### A simple and basic bootloader
* TBD

#### Adding a ring buffer to bootloader
* TBD

#### Adding a state machine to bootloader for communication between host and flash
* TBD

#### Adding a flash eraser to bootloader
* TBD

#### Adding a timeout mechanism to bootloader
* TBD

#### Adding a syncing mechanism between host and bootloader
* TBD

#### Adding an uploading firmware mechanism to bootloader
* TBD

#### Adding a checking firmware mechanism to bootloader
* TBD

#### Adding a AES-128 signing firmware mechanism to bootloader
* TBD

## Help

TBD

## Authors

Contributor names and contact info

TBD

## Version History

* 0.1
    * Initial Release

## License

This project is licensed under the FREE License - see the LICENSE.md file for details

## Acknowledgments

Inspiration, code snippets, etc.
* [mz](https://github.com/mzadfar/)