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

#include "main.h"

void init_gpio() {
    // Initialize GPIO
    gpio_init(DCJ11_POWER_CTL);
    gpio_put(DCJ11_POWER_CTL, true);  // Start with DCJ-11 SBC off
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

void tud_cdc_rx_cb(uint8_t itf)
{
    // allocate buffer for the data in the stack
    uint8_t buf[CFG_TUD_CDC_RX_BUFSIZE];

    // read the available data
    // | IMPORTANT: also do this for CDC0 because otherwise
    // | you won't be able to print anymore to CDC0
    // | next time this function is called
    uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));

    buf[count] = 0;
    cout << "Received on CDC " << static_cast<int>(itf) << ": " << buf << endl;

    tud_cdc_n_write(itf, buf, count);
    tud_cdc_n_write_flush(itf);
}

map<string, DL11*> dl11_map;

void dl11_init() {
    Device::clear_map();

    dl11_map["console"] = new DL11(0177560);
    dl11_map["tx0"] = new DL11(0176500);
    dl11_map["tx1"] = new DL11(0176510);
    dl11_map["tx2"] = new DL11(0176520);
    dl11_map["tx3"] = new DL11(0176530);
}

int main() {

    board_init();
    tusb_init();
    if (board_init_after_tusb) {
        board_init_after_tusb();
    }
    stdio_uart_init();
    init_gpio();

    dl11_init();
    start_bus_interface();

    CommandLineInterface cli;

    while (true) {
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            cli.handle_char(c);
        }

        tud_task();
    }
}
