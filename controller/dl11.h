#ifndef DL11_H
#define DL11_H

#include "device.h"
#include "pico/util/queue.h"

// DL11 register offsets
#define DL11_RXCS 0  // Receiver Control and Status
#define DL11_RXBUF 2 // Receiver Buffer
#define DL11_TXCS 4  // Transmitter Control and Status
#define DL11_TXBUF 6 // Transmitter Buffer

// Status register bits
#define DL11_RX_DONE 0x80  // Receiver Done
#define DL11_TX_RDY  0x80  // Transmitter Ready

// Data mask for 8-bit characters
#define DL11_DATA_MASK 0xFF

class DL11 : public Device {
private:
    uint16_t _rxcs;  // Receiver Control and Status
    uint16_t _txcs;  // Transmitter Control and Status
    uint16_t _rxbuf; // Receiver Buffer
    uint16_t _txbuf; // Transmitter Buffer

    queue_t* _send_queue;
    queue_t* _receive_queue;

public:
    explicit DL11(uint16_t base_address, queue_t* send_queue, queue_t* receive_queue)
    : Device(base_address, 8),
      _send_queue(send_queue),
      _receive_queue(receive_queue)
    {
        _rxcs = 0;  // Receiver ready, interrupts enabled
        _txcs = 0;   // Transmitter ready, interrupts enabled
        _rxbuf = 0;
        _txbuf = 0;
    }

    void write(uint16_t offset, uint16_t value) override;
    uint16_t read(uint16_t offset) override;
};

#endif // DL11_H