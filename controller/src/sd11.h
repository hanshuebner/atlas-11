#ifndef SD11_H
#define SD11_H

#include <array>
#include <string>

#include "device.h"
#include "ff.h"

using namespace std;

class buffer {
private:
    array<uint16_t, 512> _data;
    uint32_t _pos;
    bool _at_end;

public:
    buffer() : _data(), _pos(0), _at_end(false) {
    }

    void reset() {
        _pos = 0;
        _at_end = false;
    }

    [[nodiscard]] bool at_end() const {
        return _at_end;
    }

    array<uint16_t, 512> &get_data() { return _data; }

    void put(uint16_t x) {
        if (_at_end) {
            return;
        }
        _data[_pos++] = x;
        _at_end = _pos == _data.size();
    }

    uint16_t get() {
        if (_at_end) {
            return 0;
        }
        _at_end = _pos == _data.size() - 1;
        return _data[_pos++];
    }
};

#define COMMAND_MASK 0xc000
#define BLOCK_MASK 0x3fff

class sd11 : public Device {
private:
    typedef enum { COMMAND = 0, DATA = 2 } register_t;
    typedef enum { CMD_INIT = 0x0000, CMD_READ = 0x4000, CMD_WRITE = 0x8000, CMD_RESERVED = 0xc000 } command_t;

    command_t _current_command;
    buffer _buffer;
    string _current_file;
    uint16_t _current_block{};
    uint16_t _res{};

    void init() {
        _buffer.reset();
        _current_file = "forth.bin";
        _current_block = 0;
        _res = FR_OK;
    }
    void handle_command(uint16_t value);
    void rw(bool write);
public:
    sd11(uint16_t base_address) : Device(base_address, 4) {
        init();
    }

    void write(uint16_t offset, uint16_t value) override;
    uint16_t read(uint16_t offset) override;
};

#endif //SD11_H
