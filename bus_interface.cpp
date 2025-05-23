#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "pico/stdlib.h"
#include "bus_interface.pio.h"
#include "dcj11_gpio.h"
#include "bus_interface.h"

#include "device.h"
#include "dl11.h"

using namespace std;

// Print control signal states
static void print_control_signals(uint32_t gpio_value) {
    if (!(gpio_value & DCJ11_nALE_MASK)) cout << " nALE";
    if (!(gpio_value & DCJ11_nSCTL_MASK)) cout << " nSCTL";
    if (!(gpio_value & DCJ11_nCONT_MASK)) cout << " nCONT";
    if (!(gpio_value & DCJ11_nBUFCTL_MASK)) cout << " nBUFCTL";
    if (!(gpio_value & DCJ11_nIO_MASK)) cout << " nIO";
}

void __not_in_flash_func(cmd_iosnoop)(const vector<string> &args) {
    int capture_count = 1; // Default value
    if (args.size() > 1) {
        // Check for argument after command name
        char *endptr = nullptr;
        long parsed_count = strtol(args[1].c_str(), &endptr, 10);
        if (endptr != args[1].c_str() && *endptr == '\0' && parsed_count > 0) {
            capture_count = parsed_count;
        } else {
            cout << "Invalid capture count, using default of 10" << endl;
        }
    }

    uint32_t captures[capture_count];
    int capture_index = 0;

    cout << "Capturing " << capture_count << " bus transitions...";

    while (capture_index < capture_count) {
        uint32_t address_gpio = gpio_get_all();

        if (address_gpio & DCJ11_nALE_MASK) {
            continue;
        }

        if ((DAL_FROM_GPIO(address_gpio) & 0xF800) != 0xF800) {
            continue;
        }

        uint32_t control_gpio = gpio_get_all();
        while (control_gpio & DCJ11_nSCTL_MASK) {
            control_gpio = gpio_get_all();
        }

        captures[capture_index++] = (address_gpio & DCJ11_DAL_MASK) | (control_gpio & ~DCJ11_DAL_MASK);
        while ((gpio_get_all() & (DCJ11_nSCTL_MASK | DCJ11_nALE_MASK)) != (DCJ11_nSCTL_MASK | DCJ11_nALE_MASK)) {
            ; // wait until cycle finished
        }
    }

    cout << "done:" << endl;
    for (int i = 0; i < capture_count; i++) {
        const uint32_t value = captures[i];
        cout << (i + 1) << ": DAL: " << oct << setw(6) << setfill('0') << DAL_FROM_GPIO(value);
        print_control_signals(captures[i]);
        cout << endl;
    }
}

static void setup_dcj11_devices() {
    Device::clear_map();
    DL11 dl11(0176500);
}

PIO pio = pio0;
uint sm = 0;
uint offset = 0;

static void bus_interface_pio_start() {
    assert(pio_add_program_at_offset(pio, &bus_interface_program, offset) == offset);

    // Configure pins
    pio_gpio_init(pio, DCJ11_nBUFCTL);
    pio_gpio_init(pio, DCJ11_nALE);
    pio_gpio_init(pio, DCJ11_nSCTL);
    pio_gpio_init(pio, DCJ11_nCONT);
    pio_gpio_init(pio, DCJ11_nIO);

    // Initialize DAL pins (DAL0-15)
    for (int i = 0; i < 16; i++) {
        pio_gpio_init(pio, DCJ11_DAL0 + i);
    }

    // Configure pin directions
    pio_sm_set_pindirs_with_mask64(pio, sm,
        DCJ11_nCONT_MASK,
        DCJ11_nALE_MASK | DCJ11_nSCTL_MASK | DCJ11_nCONT_MASK |
        DCJ11_nIO_MASK | DCJ11_nBUFCTL_MASK | DCJ11_DAL_MASK);

    // Configure state machine
    pio_sm_config c = bus_interface_program_get_default_config(0);

    // Set the 'side' pins (nCONT)
    sm_config_set_sideset_pins(&c, DCJ11_nCONT);

    // Set the 'in' pins (DAL0-15, nBUFCTL)
    sm_config_set_in_pins(&c, DCJ11_DAL0);

    // Set the 'jmp' pin (nIO)
    sm_config_set_jmp_pin(&c, DCJ11_nIO);

    // Set the clock divider
    sm_config_set_clkdiv(&c, 1.0f);  // Run at full speed

    // Load the configuration and start the state machine
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

void bus_interface_pio_stop() {
    pio_sm_set_enabled(pio, sm, false);
}

[[noreturn]] void __not_in_flash_func(handle_bus)() {

    while (true) {
        // Wait for data from PIO
        uint32_t data = pio_sm_get_blocking(pio, sm);

        // Extract address and control bits
        uint16_t address = data & 0xFFF;  // 12 bits for address
        uint16_t value = data >> DCJ11_nBUFCTL;
        if (data & DCJ11_nBUFCTL_MASK) {
            // Write operation
            Device::dispatch_write(address, value);
            // Acknowledge operation
            pio_sm_put_blocking(pio, sm, value << 16);
        } else {
            // Read operation
            value = Device::dispatch_read(address);
            // Acknowledge operation and return value
            pio_sm_put_blocking(pio, sm, (value << 16) | 0xFFFF);
        }
    }
}

void cmd_bus_test(const vector<string> &args) {
    cout << "Starting bus test, press any key to stop..." << endl;

    setup_dcj11_devices();

    multicore_launch_core1(handle_bus);
    bus_interface_pio_start();

    do {
        if (multicore_fifo_rvalid()) {
            uint32_t value = multicore_fifo_pop_blocking();
            cout << "Core 1: " << hex << setw(8) << setfill('0') << value << endl;
            multicore_fifo_push_blocking(value);
        }
    } while (getchar_timeout_us(0) == PICO_ERROR_TIMEOUT);

    bus_interface_pio_stop();
    multicore_reset_core1();

    cout << "Bus test stopped." << endl;
}

