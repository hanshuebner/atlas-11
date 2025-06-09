#include "dl11.h"

void DL11::write(uint16_t offset, uint16_t value) {
    switch (offset) {
        case DL11_RXCS:
        case DL11_RXBUF:
            // Writes to receive status and data registers are ignored
            break;
        case DL11_TXCS:
            // Writes to transmit status register is ignored
            break;
        case DL11_TXBUF:
            _txbuf = value & DL11_DATA_MASK;  // Only low 8 bits are used
            // Send character to the main core via multiprocessor queue
            queue_try_add(&_send_queue, &_txbuf);
            _txcs = 0;
            break;
        default: ;
    }
}

uint16_t DL11::read(uint16_t offset) {
    switch (offset) {
        case DL11_RXCS:
            // if we can read from the fifo, then the rxcs is ready
            if (!queue_is_empty(&_receive_queue)) {
                _rxcs = DL11_RX_DONE;
                queue_remove_blocking(&_receive_queue, &_rxbuf);
            }
            return _rxcs;
        case DL11_RXBUF:
            _rxcs = 0;
            return _rxbuf;
        case DL11_TXCS:
            // if we can write to the fifo, then the txcs is ready
            if (!queue_is_full(&_send_queue)) {
                _txcs = DL11_TX_RDY;
            }
            return _txcs;
        case DL11_TXBUF:
            return _txbuf;
        default:
            return 0;
    }
}
