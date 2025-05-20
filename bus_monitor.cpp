#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdlib>
#include "pico/stdlib.h"
#include "dcj11_gpio.h"
#include "bus_monitor.h"

using namespace std;

// Convert octal string to integer
static bool parse_octal(const string &str, uint32_t &result) {
    char *endptr = nullptr;
    result = strtoul(str.c_str(), &endptr, 8);
    return endptr != str.c_str() && *endptr == '\0';
}

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

const uint16_t test_io_address = 0177700;

// Tests
void __not_in_flash_func(cmd_read_test)(const vector<string> &args) {
    cout << "Not yet implemented, but going to wiggle /CONT" << endl;

    for (int i = 0; i < 100; i++) {
        gpio_put(DCJ11_nCONT, 1);
        busy_wait_ms(10);
        gpio_put(DCJ11_nCONT, 0);
        busy_wait_ms(10);
    }
}

void __not_in_flash_func(cmd_write_test)(const vector<string> &args) {
    cout << "Write something to address " << oct << test_io_address << " on the DCJ-11 now!" << endl;

    uint16_t read_data = 0;
    bool valid = false;

    // wait for ALE to be high
    while (!(gpio_get_all() & DCJ11_nALE_MASK)) {
        ;
    }

start_cycle:
    uint32_t address_gpio = gpio_get_all();

    // Wait for ALE falling edge
    if (address_gpio & DCJ11_nALE_MASK) {
        goto start_cycle;
    }

    // Is this for our address?
    if (DAL_FROM_GPIO(address_gpio) != test_io_address) {
        goto start_cycle;
    }

    // Get the control bits
    uint32_t control_gpio = gpio_get_all();
    while (control_gpio & DCJ11_nSCTL_MASK) {
        control_gpio = gpio_get_all();
    }

    // Stretch cycle to make everything timing uncritical from here on
    gpio_put(DCJ11_nCONT, 1);

    // Is this an I/O access, loook for next cycle
    if ((control_gpio & (DCJ11_nIO_MASK | DCJ11_nBUFCTL_MASK)) == DCJ11_nBUFCTL_MASK) {
        // Write cycle J-11 to Pico
        valid = true;
        read_data = DAL_FROM_GPIO(control_gpio);
    }

    gpio_put(DCJ11_nCONT, 0);

    while ((gpio_get_all() & (DCJ11_nSCTL_MASK | DCJ11_nALE_MASK)) != (DCJ11_nSCTL_MASK | DCJ11_nALE_MASK)) {
        ; // wait until cycle finished
    }
    if (!valid) {
        goto start_cycle;
    }

    cout << "Read data: " << oct << setw(6) << setfill('0') << read_data << endl;
}
