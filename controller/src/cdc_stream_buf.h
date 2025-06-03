#ifndef CDC_STREAM_BUF_H
#define CDC_STREAM_BUF_H

#include <streambuf>
#include "cdc_peer.h"

class cdc_stream_buf : public std::streambuf, public cdc_peer {
private:
    static constexpr const char* OVERFLOW_MARKER = "\nBUFFER OVERFLOW\n";
    static constexpr size_t BUFFER_SIZE = 32 * 1024;  // 32KB buffer
    size_t _usable_buffer_size;
    char _out_buffer[BUFFER_SIZE]{};
    size_t _write_pos = 0;  // Current position in the output buffer
    size_t _read_pos = 0;   // Current position for reading from output buffer

public:
    explicit cdc_stream_buf(uint8_t itf) : cdc_peer(itf), _usable_buffer_size(BUFFER_SIZE - (strlen(OVERFLOW_MARKER) - 1)) {
        // Set up output buffer, leaving space for overflow marker
        setp(_out_buffer, _out_buffer + _usable_buffer_size - 1);
    }

    ~cdc_stream_buf() override = default;

protected:
    // Output buffer management
    std::streamsize xsputn(const char_type* s, std::streamsize count) override {
        // If we've overflowed, ignore writes until reset
        if (_write_pos == 0 && _read_pos < _usable_buffer_size) {
            return 0;
        }

        // Calculate how much space we have left
        std::streamsize space_left = _usable_buffer_size - _write_pos - 1;
        
        // If we have enough space, copy everything at once
        if (count <= space_left) {
            memcpy(_out_buffer + _write_pos, s, count);
            _write_pos += count;
            pbump(count);
            return count;
        }

        // Otherwise, copy what we can and handle overflow
        if (space_left > 0) {
            memcpy(_out_buffer + _write_pos, s, space_left);
            _write_pos += space_left;
            pbump(space_left);
        }

        // Add overflow marker
        const char* marker = OVERFLOW_MARKER;
        size_t marker_pos = _usable_buffer_size;
        while (*marker && marker_pos < BUFFER_SIZE) {
            _out_buffer[marker_pos++] = *marker++;
        }

        // Reset buffer position
        setp(_out_buffer, _out_buffer + _usable_buffer_size - 1);
        _write_pos = 0;

        return space_left;  // Return how much we actually wrote
    }

    int_type overflow(int_type ch) override {
        if (ch != traits_type::eof()) {
            // Convert single character to string and use xsputn
            char_type s[1] = { static_cast<char_type>(ch) };
            return xsputn(s, 1) == 1 ? ch : traits_type::eof();
        }
        return ch;
    }

    int sync() override {
        // Do nothing - buffer management is handled by fill_write_buf
        return 0;
    }

    void fill_write_buf() override {
        // If we've read all data, reset read position
        if (_read_pos >= _write_pos) {
            _read_pos = 0;
            _write_pos = 0;
            setp(_out_buffer, _out_buffer + _usable_buffer_size - 1);
            return;
        }

        // Calculate how much data we can copy
        size_t remaining = _write_pos - _read_pos;
        size_t to_copy = std::min(remaining, _write_buf.capacity());

        // Copy a portion of data from output buffer to write buffer
        _write_buf.assign(_out_buffer + _read_pos, _out_buffer + _read_pos + to_copy);
        _read_pos += to_copy;
    }

    void process_read_buf() override {
        // Not used for output-only streambuf
    }
};

#endif // CDC_STREAM_BUF_H