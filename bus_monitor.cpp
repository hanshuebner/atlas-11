#include <stdio.h>
#include <string>
#include <vector>
#include <cstdlib>
#include "pico/stdlib.h"
#include "dcj11_gpio.h"
#include "bus_monitor.h"

// Structure to hold captured bus state
struct bus_capture {
    uint32_t gpio_value;
};

// Convert octal string to integer
static bool parse_octal(const std::string& str, uint32_t& result) {
    char* endptr = nullptr;
    result = strtoul(str.c_str(), &endptr, 8);
    return endptr != str.c_str() && *endptr == '\0';
}

// Extract DAL value from GPIO register
static uint16_t get_dal_value(uint32_t gpio_value) {
    return (gpio_value & DCJ11_DAL_MASK) >> DCJ11_DAL0;
}

// Print control signal states
static void print_control_signals(uint32_t gpio_value) {
    std::vector<const char*> asserted;

    if (gpio_value & DCJ11_INIT_MASK) asserted.push_back("INIT");
    if (gpio_value & DCJ11_LE_MASK) asserted.push_back("LE");
    if (gpio_value & DCJ11_LBS0_MASK) asserted.push_back("LBS0");
    if (gpio_value & DCJ11_LBS1_MASK) asserted.push_back("LBS1");
    if (gpio_value & DCJ11_nWEU_MASK) asserted.push_back("nWEU");
    if (!(gpio_value & DCJ11_nWEL_MASK)) asserted.push_back("nWEL");
    if (!(gpio_value & DCJ11_nOE_MASK)) asserted.push_back("nOE");
    if (!(gpio_value & DCJ11_nSCTL_MASK)) asserted.push_back("nSCTL");
    if (!(gpio_value & DCJ11_nCONT_MASK)) asserted.push_back("nCONT");
    if (gpio_value & DCJ11_HALT_MASK) asserted.push_back("HALT");

    for (size_t i = 0; i < asserted.size(); i++) {
        printf("%s%s", i == 0 ? "" : " ", asserted[i]);
    }
    printf("\n");
}

void print_bus(uint32_t value) {
    printf("DAL: %06o ", get_dal_value(value));
    print_control_signals(value);
}

void cmd_bus(const std::vector<std::string>& args) {
    int capture_count = 10;  // Default value
    if (args.size() > 1) {  // Check for argument after command name
        char* endptr = nullptr;
        long parsed_count = strtol(args[1].c_str(), &endptr, 10);
        if (endptr != args[1].c_str() && *endptr == '\0' && parsed_count > 0) {
            capture_count = parsed_count;
        } else {
            printf("Invalid capture count, using default of 10\n");
        }
    }

    std::vector<bus_capture> captures(capture_count);
    int capture_index = 0;

    printf("Capturing %d bus transitions...\n", capture_count);

    while (capture_index < capture_count) {
        uint32_t current_gpio = gpio_get_all();

        if (current_gpio & DCJ11_LE_MASK) {
            captures[capture_index].gpio_value = current_gpio;
            capture_index++;
            while (gpio_get_all() & DCJ11_LE_MASK) {
                ; // wait until LE deasserted
            }
        }
    }

    // Print all captures
    printf("\nCaptured %d bus transitions:\n", capture_count);
    for (int i = 0; i < capture_count; i++) {
        printf("%d: ", i + 1);
        print_bus(captures[i].gpio_value);
    }
}