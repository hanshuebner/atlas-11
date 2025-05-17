# Atlas-11 Firmware

This is the firmware for the Atlas-11 project running on an RP2350 board (PGA2350) using the Raspberry Pi Pico SDK.

## Prerequisites

1. Install the Raspberry Pi Pico SDK:
   ```bash
   # Clone the SDK repository
   git clone https://github.com/raspberrypi/pico-sdk.git
   cd pico-sdk
   git submodule update --init
   cd ..
   ```

2. Set up the PICO_SDK_PATH environment variable:
   ```bash
   export PICO_SDK_PATH=/path/to/pico-sdk
   ```

3. The project is configured for the RP2350 platform and PGA2350 board. These settings are defined in CMakeLists.txt:
   ```cmake
   set(PICO_PLATFORM "rp2350")
   set(PICO_BOARD "pga2350")
   ```

## Building

1. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

2. Generate build files:
   ```bash
   cmake ..
   ```

3. Build the project:
   ```bash
   make
   ```

## Flashing

1. Put the RP2350 board in bootloader mode by holding the BOOTSEL button while connecting it to USB.

2. Copy the generated UF2 file to the mounted RPI-RP2 drive:
   ```bash
   cp atlas-11.uf2 /Volumes/RPI-RP2/
   ```

## Usage

1. Connect to the board using a serial terminal (e.g., `screen` or `minicom`):
   ```bash
   screen /dev/tty.usbmodem* 115200
   ```

2. Available commands:
   - `on` - Set GPIO pin 2 high
   - `off` - Set GPIO pin 2 low
   - `help` - Show help message
   - `exit` - Exit the program

## Hardware

- Board: RP2350 (PGA2350)
- GPIO Pin 2 is used for output control 