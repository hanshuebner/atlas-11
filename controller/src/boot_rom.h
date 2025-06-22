#ifndef BOOT_ROM_H
#define BOOT_ROM_H

#include "device.h"

using namespace std;

class boot_rom : public Device {
    const uint16_t *const _contents;

public:
    explicit boot_rom(uint16_t base_address, const uint16_t contents[256])
        : Device(base_address, 256),
          _contents(contents) {
    };

    void write(uint16_t offset, uint16_t value) override {
        // Ignore writes
    };
    uint16_t read(uint16_t offset) override {
        printf("boot_rom::read: 0%06o => 0%06o\n", offset >> 1, _contents[offset >> 1]);
        return _contents[offset >> 1];
    };
};

#endif //BOOT_ROM_H
