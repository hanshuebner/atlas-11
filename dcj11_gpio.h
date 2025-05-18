#ifndef DCJ11_GPIO_H
#define DCJ11_GPIO_H

// UART pins
#define DCJ11_UART0_TX    0  // UART0 Transmit
#define DCJ11_UART0_RX    1  // UART0 Receive

// Power Control
#define DCJ11_POWER_CTL   2  // Controls power to DCJ11 SBC

// Data Address Lines
#define DCJ11_DAL0        5  // Data Address Line 0
#define DCJ11_DAL1        6  // Data Address Line 1
#define DCJ11_DAL2        7  // Data Address Line 2
#define DCJ11_DAL3        8  // Data Address Line 3
#define DCJ11_DAL4        9  // Data Address Line 4
#define DCJ11_DAL5        10 // Data Address Line 5
#define DCJ11_DAL6        11 // Data Address Line 6
#define DCJ11_DAL7        12 // Data Address Line 7
#define DCJ11_DAL8        13 // Data Address Line 8
#define DCJ11_DAL9        14 // Data Address Line 9
#define DCJ11_DAL10       15 // Data Address Line 10
#define DCJ11_DAL11       16 // Data Address Line 11
#define DCJ11_DAL12       17 // Data Address Line 12
#define DCJ11_DAL13       18 // Data Address Line 13
#define DCJ11_DAL14       19 // Data Address Line 14
#define DCJ11_DAL15       20 // Data Address Line 15

// Control signals
#define DCJ11_INIT        21 // Initialize signal
#define DCJ11_LE          22 // Latch Enable
#define DCJ11_LBS0        23 // Lower Byte Select 0
#define DCJ11_LBS1        24 // Lower Byte Select 1
#define DCJ11_nWEU        25 // Write Enable Upper (negative logic)
#define DCJ11_nWEL        26 // Write Enable Lower (negative logic)
#define DCJ11_nOE         27 // Output Enable (negative logic)
#define DCJ11_nSCTL       28 // System Control (negative logic)
#define DCJ11_nCONT       29 // Continue signal (negative logic)
#define DCJ11_HALT        30 // Halt signal

// SD Card SPI Interface
#define DCJ11_SDCARD_MOSI 43 // SPI1 TX
#define DCJ11_SDCARD_MISO 44 // SPI1 RX
#define DCJ11_SDCARD_CS   45 // SPI1 CSn
#define DCJ11_SDCARD_SCK  46 // SPI1 SCK

// Bit masks for full register access
#define DCJ11_DAL_MASK    0x001FFFE0  // DAL0-15 (bits 5-20)
#define DCJ11_CTL_MASK    0x7FE00000  // Control signals (bits 21-30)

// Individual control signal masks
#define DCJ11_INIT_MASK   (1 << DCJ11_INIT)
#define DCJ11_LE_MASK     (1 << DCJ11_LE)
#define DCJ11_LBS0_MASK   (1 << DCJ11_LBS0)
#define DCJ11_LBS1_MASK   (1 << DCJ11_LBS1)
#define DCJ11_nWEU_MASK   (1 << DCJ11_nWEU)
#define DCJ11_nWEL_MASK   (1 << DCJ11_nWEL)
#define DCJ11_nOE_MASK    (1 << DCJ11_nOE)
#define DCJ11_nSCTL_MASK  (1 << DCJ11_nSCTL)
#define DCJ11_nCONT_MASK  (1 << DCJ11_nCONT)
#define DCJ11_HALT_MASK   (1 << DCJ11_HALT)

#endif // DCJ11_GPIO_H 