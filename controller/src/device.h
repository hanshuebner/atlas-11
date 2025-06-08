#ifndef DEVICE_H
#define DEVICE_H
#include <cstdint>

#define IO_PAGE_SIZE 4096
#define IO_OFFSET_MASK (IO_PAGE_SIZE - 1)
#define IO_PAGE_MASK (~IO_OFFSET_MASK)

class Device {
private:
    static Device* _map[IO_PAGE_SIZE];

public:
    static void clear_map() {
        for (auto & i : _map) {
            i = nullptr;
        }
    }

    static inline void dispatch_write(uint16_t address, uint16_t value) {
        Device* handler = Device::_map[address & IO_OFFSET_MASK];
        if (handler != nullptr) {
            handler->write(address & handler->_device_offset_mask, value);
        }
    }
    static inline uint16_t dispatch_read(uint16_t address) {
        Device* handler = Device::_map[address & IO_OFFSET_MASK];
        if (handler != nullptr) {
            return handler->read(address & handler->_device_offset_mask);
        }
        return address; // for testing
        return 0;
    }
    uint16_t _device_offset_mask;

    Device(uint16_t base_address, uint16_t size) {
        const uint16_t base_offset = base_address & IO_OFFSET_MASK;
        for (uint16_t i = 0; i < size; i++) {
            _map[base_offset + i] = this;
        }
        _device_offset_mask = size - 1;
    };
    virtual ~Device() = default;

    virtual void write(uint16_t offset, uint16_t value) = 0;
    virtual uint16_t read(uint16_t offset) = 0;
};

#endif //DEVICE_H
