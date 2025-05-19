#include <stdio.h>
#include <string>
#include <vector>
#include <cstdlib>
#include "pico/stdlib.h"
#include "dcj11_gpio.h"
#include "bus_monitor.h"

// Convert octal string to integer
static bool parse_octal(const std::string& str, uint32_t& result) {
    char* endptr = nullptr;
    result = strtoul(str.c_str(), &endptr, 8);
    return endptr != str.c_str() && *endptr == '\0';
}

// Print control signal states
static void print_control_signals(uint32_t gpio_value) {
    std::vector<const char*> asserted;

    if (!(gpio_value & DCJ11_nALE_MASK)) asserted.push_back("nALE");
    if (!(gpio_value & DCJ11_nSCTL_MASK)) asserted.push_back("nSCTL");
    if (!(gpio_value & DCJ11_nCONT_MASK)) asserted.push_back("nCONT");
    if (!(gpio_value & DCJ11_nBUFCTL_MASK)) asserted.push_back("nBUFCTL");
    if (!(gpio_value & DCJ11_nIO_MASK)) asserted.push_back("nIO");

    for (size_t i = 0; i < asserted.size(); i++) {
        printf("%s%s", i == 0 ? "" : " ", asserted[i]);
    }
}

void __not_in_flash_func(cmd_iosnoop)(const std::vector<std::string>& args) {
    int capture_count = 1;  // Default value
    if (args.size() > 1) {  // Check for argument after command name
        char* endptr = nullptr;
        long parsed_count = strtol(args[1].c_str(), &endptr, 10);
        if (endptr != args[1].c_str() && *endptr == '\0' && parsed_count > 0) {
            capture_count = parsed_count;
        } else {
            printf("Invalid capture count, using default of 10\n");
        }
    }

    uint32_t captures[capture_count];
    int capture_index = 0;

    printf("Capturing %d bus transitions...", capture_count);

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

    printf("done:\n");
    for (int i = 0; i < capture_count; i++) {
        const uint32_t value = captures[i];
        printf("%d: DAL: %06o ", i + 1, DAL_FROM_GPIO(value));
        print_control_signals(captures[i]);
        putchar('\n');
    }
}