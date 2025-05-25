# Atlas-11 DCJ-11 I/O controller

This is the firmware for the Atlas-11 project running on an RP2350 board (PGA2350) using the Raspberry Pi Pico SDK.

## Hardware

- Board: RP2350 (PGA2350)

### GPIO / PIN usage

| GPIO Pin | Function | Description |
|----------|----------|-------------|
| GPIO 0   | UART0 TX | UART0 Transmit |
| GPIO 1   | UART0 RX | UART0 Receive |
| GPIO 2   | Power Control | Controls power to DCJ11 SBC |
| GPIO 5   | DAL0 | Data Address Line 0 |
| GPIO 6   | DAL1 | Data Address Line 1 |
| GPIO 7   | DAL2 | Data Address Line 2 |
| GPIO 8   | DAL3 | Data Address Line 3 |
| GPIO 9   | DAL4 | Data Address Line 4 |
| GPIO 10  | DAL5 | Data Address Line 5 |
| GPIO 11  | DAL6 | Data Address Line 6 |
| GPIO 12  | DAL7 | Data Address Line 7 |
| GPIO 13  | DAL8 | Data Address Line 8 |
| GPIO 14  | DAL9 | Data Address Line 9 |
| GPIO 15  | DAL10 | Data Address Line 10 |
| GPIO 16  | DAL11 | Data Address Line 11 |
| GPIO 17  | DAL12 | Data Address Line 12 |
| GPIO 18  | DAL13 | Data Address Line 13 |
| GPIO 19  | DAL14 | Data Address Line 14 |
| GPIO 20  | DAL15 | Data Address Line 15 |
| GPIO 21  | nBUFCTL | Buffer Control (Read/Write) |
| GPIO 22  | HALT | Halt signal |
| GPIO 23  | nALE | Address Latch Enable (negative logic) |
| GPIO 24  | nSCTL | Stretch Control |
| GPIO 25  | nCONT | Continue signal (negative logic) |
| GPIO 26  | INIT | Initialize signal |
| GPIO 27  | nIO | I/O access |
| GPIO 43 | SPI1_TX | SDCARD MOSI |
| GPIO 44 | SPI1_RX | SDCARD MISO |
| GPIO 45 | SPI1_CSn | SDCARD CS |
| GPIO 46 | SPI1_SCK | SDCARD SCK |
