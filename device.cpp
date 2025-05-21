//
// Created by Hans Hübner on 21.05.25.
//

#include "device.h"

void Device::dispatch_write(uint16_t address, uint16_t value) {
    Device* handler = Device::_map[address & IO_OFFSET_MASK];
    if (handler != nullptr) {
        handler->write(address & handler->_device_offset_mask, value);
    }
}

uint16_t Device::dispatch_read(uint16_t address) {
    Device* handler = Device::_map[address & IO_OFFSET_MASK];
    if (handler != nullptr) {
        return handler->read(address & handler->_device_offset_mask);
    }
    return 0;
}
