
#include "sd11.h"
#include "fs.h"
#include "ff.h"

void
sd11::rw(bool write) {
    with_sdcard_mounted([this, write]() {
        FIL file;
        _res = f_open(&file, _current_file.c_str(), FA_READ);
        if (_res != FR_OK) {
            return;
        }

        _res = f_lseek(&file, _current_block * 1024);
        if (_res != FR_OK) {
            return;
        }

        if (write) {
            UINT bytes_written;
            _res = f_write(&file, _buffer.get_data().data(), 1024, &bytes_written);
        } else {
            UINT bytes_read;
            _res = f_read(&file, _buffer.get_data().data(), 1024, &bytes_read);
        }

        _buffer.reset();
    });
}

void
sd11::handle_command(uint16_t value) {
    _current_command = static_cast<command_t>(value & COMMAND_MASK);
    _current_block = value & BLOCK_MASK;
    switch (_current_command) {
        case CMD_INIT:
            init();
            break;
        case CMD_READ:
            rw(false);
            break;
        case CMD_WRITE:
            _buffer.reset();
            break;
        case CMD_RESERVED:
            // do nothing
            break;
    }
}

void
sd11::write(uint16_t offset, uint16_t value) {
    switch (offset) {
        case COMMAND:
            handle_command(value);
            break;
        case DATA:
            if (_current_command == CMD_WRITE) {
                _buffer.put(value);
                if (_buffer.at_end()) {
                    rw(true);
                }
            }
            break;
        default:
            // do nothing
            break;
    }
}

uint16_t
sd11::read(uint16_t offset) {
    switch (offset) {
        case COMMAND:
            return _res == FR_OK ? 0 : 1;
        case DATA:
            if (_current_command == CMD_READ) {
                return _buffer.get();
            }
        default:
            return 0;
    }
}
