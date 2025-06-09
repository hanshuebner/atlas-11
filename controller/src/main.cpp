#include <iostream>
#include <map>
#include <bsp/board_api.h>
#include <pico/stdlib.h>
#include <pico/bootrom.h>
#include <tusb.h>

#include "bus_interface.h"
#include "dcj11_gpio.h"
#include "cli.h"
#include "dl11.h"
#include "config.h"

#include "main.h"

#include "boot_rom.h"
#include "sd11.h"

void init_gpio() {
    // Initialize GPIO
    gpio_init(DCJ11_POWER_CTL);
    gpio_put(DCJ11_POWER_CTL, false);  // Start with DCJ-11 SBC off
    gpio_set_dir(DCJ11_POWER_CTL, GPIO_OUT);

    gpio_init(DCJ11_HALT);
    gpio_set_dir(DCJ11_HALT, GPIO_OUT);
    gpio_put(DCJ11_HALT, false);  // Start with HALT low

    // Initialize Data Address Lines (DAL0-15) as inputs
    for (int pin = DCJ11_DAL0; pin <= DCJ11_DAL15; pin++) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
    }

    const int control_inputs[] = {
        DCJ11_INIT, DCJ11_nALE, DCJ11_nSCTL, DCJ11_nBUFCTL, DCJ11_nIO,
    };

    for (int pin : control_inputs) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
    }

    constexpr int control_outputs[] = {
        DCJ11_HALT, DCJ11_nCONT
    };

    for (int pin : control_outputs) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
    }
}

constexpr uint16_t dd5boot_words[256] = {
    0x15c3, 0xfd44, 0x10c1, 0x0a8b, 0x15c2, 0xffff, 0x8bcb, 0x80fe,
    0x90b3, 0x0002, 0x8a02, 0x00c2, 0x02f9, 0x8bcb, 0x80fe, 0x0a0b,
    0x15c0, 0x0004, 0x0be1, 0x45c0, 0x0010, 0x8bcb, 0x80fe, 0x9033,
    0x0002, 0x0cc0, 0x02f8, 0x8bf1, 0xfffe, 0x80fd, 0x9252, 0x25c2,
    0x0200, 0x82f9, 0x0a07, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

typedef array<uint8_t, CFG_TUD_CDC_TX_BUFSIZE> cdc_tx_buffer_t;
vector<cdc_tx_buffer_t> tx_buffers(CFG_TUD_CDC);
vector<DL11*> dl11s(CFG_TUD_CDC);
boot_rom* dd5boot;
sd11* sdcard;

void dcj11_device_init() {
    Device::clear_map();

    dl11s[0] = new DL11(0177560);
    dl11s[1] = new DL11(0176500);
    dl11s[2] = new DL11(0176510);
    dl11s[3] = new DL11(0176520);
    dl11s[4] = new DL11(0176530);

    dd5boot = new boot_rom(0173000, dd5boot_words);
    sdcard = new sd11(0177600);
}

void cdc_task() {
    uint8_t rx_buffer[CFG_TUD_CDC_RX_BUFSIZE+1];

    for (uint8_t itf = 0; itf < CFG_TUD_CDC; itf++) {
        auto receive_queue = dl11s[itf]->get_receive_queue();
        if (tud_cdc_n_available(itf) && queue_is_empty(receive_queue)) {
            uint32_t count = tud_cdc_n_read(itf, rx_buffer, sizeof(rx_buffer));
            for (int i = 0; i < count; i++) {
                queue_add_blocking(receive_queue, &rx_buffer[i]);
            }
        }
        auto send_queue = dl11s[itf]->get_send_queue();
        if (!queue_is_empty(send_queue) && !tud_cdc_n_write_flush(itf)) {
            auto tx_buffer = tx_buffers[itf];
            int count = 0;
            for (; count < CFG_TUD_CDC_TX_BUFSIZE; count++) {
                if (!queue_try_remove(send_queue, &tx_buffer[count])) {
                    break;
                }
            }
            tud_cdc_n_write(itf, tx_buffer.data(), count);
            tud_cdc_n_write_flush(itf);
        }
    }
}

int main() {

    board_init();
    tusb_init();
    if (board_init_after_tusb) {
        board_init_after_tusb();
    }
    stdio_uart_init();
    init_gpio();

    auto config = read_config();

    dcj11_device_init();
    start_bus_interface();

    CommandLineInterface cli;

    while (true) {
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            cli.handle_char(c);
        }

        tud_task();
        cdc_task();
    }
}
