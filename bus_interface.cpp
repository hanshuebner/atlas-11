#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "pico/stdlib.h"
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

[[noreturn]] void __not_in_flash_func(handle_bus)() {
    DL11 dl11(0176500);

    // wait for ALE to be high before looping
    while (!(gpio_get_all() & DCJ11_nALE_MASK)) {
        ;
    }

    while (true) {
        uint32_t address_gpio = gpio_get_all();

        // Wait for ALE falling edge
        if (address_gpio & DCJ11_nALE_MASK) {
            continue;
        }

        // Get the control bits
        uint32_t control_gpio = gpio_get_all();
        while (!(control_gpio & DCJ11_nALE_MASK)) {
            control_gpio = gpio_get_all();
        }

        // I/O space access?
        if (!(control_gpio & DCJ11_nIO_MASK)) {
            // Stretch cycle to make everything timing uncritical from here on
            gpio_put(DCJ11_nCONT, true);

            const uint16_t address = DAL_FROM_GPIO(address_gpio);
            if (IS_READ_ACCESS(control_gpio)) {
                gpio_set_dir_out_masked(DCJ11_DAL_MASK);
                uint16_t value = Device::dispatch_read(address);
                gpio_put_masked(DCJ11_DAL_MASK, GPIO_FROM_DAL(value));
            } else {
                Device::dispatch_write(address, DAL_FROM_GPIO(control_gpio));
            }

            // End stretched cycle
            gpio_put(DCJ11_nCONT, false);
        }


        while ((gpio_get_all() & (DCJ11_nSCTL_MASK | DCJ11_nALE_MASK)) != (DCJ11_nSCTL_MASK | DCJ11_nALE_MASK)) {
            tight_loop_contents(); // wait until cycle finished
        }

        gpio_set_dir_in_masked(DCJ11_DAL_MASK);
    }
}

void cmd_bus_test(const vector<string> &args) {
    cout << "Starting bus test, press any key to stop..." << endl;

    multicore_launch_core1(handle_bus);
    do {
        if (multicore_fifo_rvalid()) {
            uint32_t value = multicore_fifo_pop_blocking();
            cout << "Core 1: " << hex << setw(8) << setfill('0') << value << endl;
            multicore_fifo_push_blocking(value);
        }
    } while (getchar_timeout_us(0) == PICO_ERROR_TIMEOUT);
    multicore_reset_core1();
    cout << "Bus test stopped." << endl;
}

