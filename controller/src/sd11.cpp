
#include "sd11.h"
#include "fs.h"
#include "ff.h"

void
sd11::rw(bool write) {
    with_sdcard_mounted([this, write]() {

        _buffer.reset();

        printf("sd11::rw %s file %s block %d ", write ? "write" : "read", _current_file.c_str(), _current_block);
        FIL file;
        _res = f_open(&file, _current_file.c_str(), FA_READ);
        if (_res != FR_OK) {
            printf("open: %s\n", f_result_to_str(_res));
            return;
        }

        _res = f_lseek(&file, _current_block * 1024);
        if (_res != FR_OK) {
            printf("lseek: %s\n", f_result_to_str(_res));
            return;
        }

        if (write) {
            UINT bytes_written;
            _res = f_write(&file, _buffer.get_data().data(), 1024, &bytes_written);
            if (bytes_written != 1024) {
                printf("failed (f_write returned %d)\n", bytes_written);
                _res = FR_EXT_BLOCK_WRITE_FAILED;
            } else {
                printf("OK\n");
            }
        } else {
            UINT bytes_read;
            _res = f_read(&file, _buffer.get_data().data(), 1024, &bytes_read);
            if (bytes_read == 0) {
                printf("failed\n");
                _res = FR_EXT_BLOCK_READ_FAILED;
            } else if (bytes_read != 1024) {
                printf("only %d bytes read\n", bytes_read);
            } else {
                printf("OK\n");
            }
        }
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
                if (_buffer.at_end() && _res == FR_OK) {
                    _current_block++;
                    rw(false);
                }
                return _buffer.get();
            }
        default:
            return 0;
    }
}
