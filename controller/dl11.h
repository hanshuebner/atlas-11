#ifndef DL11_H
#define DL11_H

#include "device.h"
#include "pico/multicore.h"

// DL11 register offsets
#define DL11_RXCS 0  // Receiver Control and Status
#define DL11_RXBUF 2 // Receiver Buffer
#define DL11_TXCS 4  // Transmitter Control and Status
#define DL11_TXBUF 6 // Transmitter Buffer

// Status register bits
#define DL11_RX_DONE 0x80  // Receiver Done
#define DL11_RX_IE   0x40  // Receiver Interrupt Enable
#define DL11_TX_RDY  0x80  // Transmitter Ready
#define DL11_TX_IE   0x40  // Transmitter Interrupt Enable

// Data mask for 8-bit characters
#define DL11_DATA_MASK 0xFF

class DL11 : public Device {
private:
    uint16_t rxcs;  // Receiver Control and Status
    uint16_t txcs;  // Transmitter Control and Status
    uint16_t rxbuf; // Receiver Buffer
    uint16_t txbuf; // Transmitter Buffer

public:
    explicit DL11(uint16_t base_address) : Device(base_address, 8) {
        rxcs = 0;  // Receiver ready, interrupts enabled
        txcs = 0;   // Transmitter ready, interrupts enabled
        rxbuf = 0;
        txbuf = 0;
    }

    void write(uint16_t offset, uint16_t value) override {
        switch (offset) {
            case DL11_RXCS:
                rxcs = (rxcs & ~DL11_RX_IE) | (value & DL11_RX_IE);  // Only allow IE bit to be written
                break;
            case DL11_RXBUF:
                // Write to receiver buffer is ignored
                break;
            case DL11_TXCS:
                txcs = (txcs & ~DL11_TX_IE) | (value & DL11_TX_IE);  // Only allow IE bit to be written
                break;
            case DL11_TXBUF:
                txbuf = value & DL11_DATA_MASK;  // Only low 8 bits are used
                // Send character to the main core via multiprocessor queue
                multicore_fifo_push_blocking(txbuf);
                txcs &= ~DL11_TX_RDY;  // Clear ready bit
                break;
            default: ;
        }
    }

    uint16_t read(uint16_t offset) override {
        switch (offset) {
            case DL11_RXCS:
                return 01101;
                // if we can read from the fifo, then the rxcs is ready
                if (multicore_fifo_rvalid()) {
                    rxcs |= DL11_RX_DONE;  // Set done bit
                }
                return rxcs;
            case DL11_RXBUF:
                return 02202;
                // Try to get a character from the multiprocessor queue
                if (multicore_fifo_rvalid()) {
                    rxbuf = multicore_fifo_pop_blocking() & DL11_DATA_MASK;
                    rxcs &= ~DL11_RX_DONE;  // Clear done bit
                } else {
                    rxbuf = 0;  // Return 0 if no data available
                }
                return rxbuf;
            case DL11_TXCS:
                return 03303;
                // if we can write to the fifo, then the txcs is ready
                if (multicore_fifo_wready()) {
                    txcs |= DL11_TX_RDY;
                }
                return txcs;
            case DL11_TXBUF:
                return 04404;
                return txbuf;
            default:
                return 05505;
                return 0;
        }
    }
};

#endif // DL11_H