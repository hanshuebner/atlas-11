#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "console.hpp"
#include "dcj11_gpio.h"
#include "bus_monitor.h"
#include "fs.h"

// Command handler function type with parameters
using cmd_handler_t = void(*)(const std::vector<std::string>&);

// Command structure
struct cmd_entry {
    std::string name;
    cmd_handler_t handler;
    std::string usage;
};

// Command handlers
static void cmd_on(const std::vector<std::string>& args) {
    gpio_put(DCJ11_POWER_CTL, 1);
    printf("DJC11 SBC powered on\n");
}

static void cmd_off(const std::vector<std::string>& args) {
    gpio_put(DCJ11_POWER_CTL, 0);
    printf("DJC11 SBC powered off\n");
}

static void cmd_halt(const std::vector<std::string>& args) {
    gpio_put(DCJ11_HALT, 1);
    sleep_ms(10);  // Hold HALT high for 10ms
    gpio_put(DCJ11_HALT, 0);
    printf("HALT signal asserted for 10ms\n");
}

static void cmd_help(const std::vector<std::string>& args);

static void cmd_update(const std::vector<std::string>& args) {
    printf("Rebooting into firmware update mode...\n");
    sleep_ms(100);  // Give time for message to be printed
    reset_usb_boot(0, 0);  // Reboot into USB bootloader
}

static void cmd_console(const std::vector<std::string>& args) {
    cmd_on(args);
    console_mode();
}

// Command table
static const std::vector<cmd_entry> cmd_table = {
    {"iosnoop", cmd_iosnoop, "Monitor I/O access on bus (e.g. 'iosnoop 20' for 20 captures, default 10)"},
    { "write-test", cmd_write_test, "Perform a bus write test (J11 -> Pico)"},
    { "read-test", cmd_read_test, "Perform a bus read test (Pico -> J11)"},
    {"console", cmd_console, "Enter UART console mode (38400 8N1) (powers on, too)"},
    {"halt", cmd_halt, "Assert HALT signal for 10ms"},
    {"ls", cmd_ls, "List files on SD card"},
    {"on", cmd_on, "Power on the DJC11 SBC"},
    {"off", cmd_off, "Power off the DJC11 SBC"},
    {"update", cmd_update, "Reboot into firmware update mode"},
    {"help", cmd_help, "Show help message"}
};

// Command lookup table
static std::unordered_map<std::string, const cmd_entry*> cmd_map;

// Initialize command map with abbreviations
static void init_cmd_map() {
    // First add all full command names
    for (const auto& cmd : cmd_table) {
        cmd_map[cmd.name] = &cmd;
    }

    // Track non-unique prefixes
    std::vector<std::string> non_unique;

    // Try all possible prefixes for each command
    for (const auto& cmd : cmd_table) {
        for (size_t i = 1; i < cmd.name.length(); i++) {
            std::string prefix = cmd.name.substr(0, i);

            // Check if this prefix is already in the map
            if (cmd_map.count(prefix)) {
                non_unique.push_back(prefix);
            } else {
                cmd_map[prefix] = &cmd;
            }
        }
    }

    // Remove all non-unique prefixes
    for (const auto& prefix : non_unique) {
        cmd_map.erase(prefix);
    }
}

void cmd_help(const std::vector<std::string>& args) {
    printf("\nAtlas-11 GPIO Control Interface\n");
    printf("Commands:\n");

    // Print help using the command table
    for (const auto& cmd : cmd_table) {
        printf("  %-8s - %s\n", cmd.name.c_str(), cmd.usage.c_str());
    }
}

// Split string into vector of strings
static std::vector<std::string> split_string(const std::string& str) {
    std::vector<std::string> result;
    std::istringstream iss(str);
    std::string token;

    while (iss >> token) {
        result.push_back(token);
    }

    return result;
}

// Read a line of input with basic line editing capabilities
// Returns true if a command was entered, false if empty line
bool read_line(std::string& buffer) {
    int c;
    buffer.clear();

    printf("\n> ");

    while (true) {
        c = getchar();

        // Handle enter key
        if (c == '\r' || c == '\n') {
            printf("\n");
            return !buffer.empty();  // Return false for empty line
        }

        // Handle backspace
        if (c == '\b' || c == 127) {
            if (!buffer.empty()) {
                buffer.pop_back();
                printf("\b \b");  // Move back, print space, move back again
            }
            continue;
        }

        // Handle regular character
        if (c >= 32 && c <= 126) {  // Printable ASCII
            buffer.push_back(c);
            putchar(c);
        }
    }
}

void init_gpio() {
    // Initialize GPIO
    gpio_init(DCJ11_POWER_CTL);
    gpio_set_dir(DCJ11_POWER_CTL, GPIO_OUT);
    gpio_put(DCJ11_POWER_CTL, 0);  // Start with pin low

    gpio_init(DCJ11_HALT);
    gpio_set_dir(DCJ11_HALT, GPIO_OUT);
    gpio_put(DCJ11_HALT, 0);  // Start with HALT low

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

    const int control_outputs[] = {
        DCJ11_HALT, DCJ11_nCONT
    };

    for (int pin : control_outputs) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
    }
}

int main() {
    // Initialize stdio
    stdio_init_all();

    init_gpio();

    // Initialize command map
    init_cmd_map();

    // Wait for USB CDC to be ready
    sleep_ms(1000);

    printf("Atlas-11 Firmware Starting...\n");
    cmd_help({});

    std::string cmd;

    while (true) {
        if (!read_line(cmd)) {
            cmd_help({});
            continue;
        }

        // Split command into words
        std::vector<std::string> args = split_string(cmd);
        if (args.empty()) continue;

        // Look up command in map
        auto it = cmd_map.find(args[0]);
        if (it != cmd_map.end()) {
            it->second->handler(args);
        } else {
            printf("Unknown command. Type 'help' for available commands.\n");
        }
    }

    return 0;
}
