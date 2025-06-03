#ifndef CDC_PEER_H
#define CDC_PEER_H

#include <algorithm>
#include <pico/util/queue.h>

class cdc_peer {

    uint8_t _itf;

protected:
    vector<uint8_t> _read_buf;
    vector<uint8_t> _write_buf;
public:
    explicit cdc_peer(uint8_t itf)
        : _itf(itf),
          _read_buf(CFG_TUD_CDC_RX_BUFSIZE),
          _write_buf(CFG_TUD_CDC_TX_BUFSIZE)
    {}
    virtual ~cdc_peer() = default;

    void read_from_host() {
        uint32_t count = tud_cdc_n_read(_itf, _read_buf.data(), sizeof(_read_buf.capacity()));
        _read_buf.resize(count);
    }

    virtual void process_read_buf() = 0;
    virtual void fill_write_buf() = 0;

    void poll() {
        if (_read_buf.empty() && tud_cdc_n_available(_itf)) {
            read_from_host();
        }
        if (!_read_buf.empty()) {
            process_read_buf();
        }
        if (_write_buf.empty() && (tud_cdc_n_write_flush(_itf) == 0)) {
            fill_write_buf();
        }
        if (!_write_buf.empty()) {
            tud_cdc_n_write(_itf, _write_buf.data(), _write_buf.size());
            _write_buf.clear();
        }
    }
};

#endif //CDC_PEER_H
