#ifndef DEVICE_H
#define DEVICE_H
#include <cstdint>

#define IO_PAGE_SIZE 8192
#define IO_OFFSET_MASK (IO_PAGE_SIZE - 1)
#define IO_PAGE_MASK (~IO_OFFSET_MASK)

class Device {
private:
    static Device* _map[IO_PAGE_SIZE];

public:
    static void dispatch_write(uint16_t address, uint16_t value);
    static uint16_t dispatch_read(uint16_t address);
    uint16_t _device_offset_mask;

    Device(uint16_t base_address, uint8_t size) {
        const uint16_t base_offset = base_address & IO_OFFSET_MASK;
        for (uint8_t i = 0; i < size; i++) {
            _map[base_offset + i] = this;
        }
        _device_offset_mask = ~(size - 1);
    };
    virtual ~Device() = default;

    virtual void write(uint16_t offset, uint16_t value) = 0;
    virtual uint16_t read(uint16_t offset) = 0;
};

#endif //DEVICE_H
