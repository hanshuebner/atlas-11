#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "console.hpp"

#define GPIO_PIN 2

// Command handler function type
using cmd_handler_t = void(*)();

// Command structure
struct cmd_entry {
    std::string name;
    cmd_handler_t handler;
    std::string usage;
};

// Command handlers
static void cmd_on() {
    gpio_put(GPIO_PIN, 1);
    printf("DJC11 SBC powered on\n");
}

static void cmd_off() {
    gpio_put(GPIO_PIN, 0);
    printf("DJC11 SBC powered off\n");
}

static void cmd_help();

static void cmd_update() {
    printf("Rebooting into firmware update mode...\n");
    sleep_ms(100);  // Give time for message to be printed
    reset_usb_boot(0, 0);  // Reboot into USB bootloader
}

static void cmd_console() {
    console_mode();
}

// Command table
static const std::vector<cmd_entry> cmd_table = {
    {"on", cmd_on, "Power on the DJC11 SBC"},
    {"off", cmd_off, "Power off the DJC11 SBC"},
    {"help", cmd_help, "Show help message"},
    {"update", cmd_update, "Reboot into firmware update mode"},
    {"console", cmd_console, "Enter UART console mode (38400 8N1)"}
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

void cmd_help() {
    printf("\nAtlas-11 GPIO Control Interface\n");
    printf("Commands:\n");
    
    // Print help using the command table
    for (const auto& cmd : cmd_table) {
        printf("  %-8s - %s\n", cmd.name.c_str(), cmd.usage.c_str());
    }
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

int main() {
    // Initialize stdio
    stdio_init_all();
    
    // Initialize GPIO
    gpio_init(GPIO_PIN);
    gpio_set_dir(GPIO_PIN, GPIO_OUT);
    gpio_put(GPIO_PIN, 0);  // Start with pin low
    
    // Initialize command map
    init_cmd_map();
    
    // Wait for USB CDC to be ready
    sleep_ms(1000);
    
    printf("Atlas-11 Firmware Starting...\n");
    cmd_help();
    
    std::string cmd;
    
    while (true) {
        if (!read_line(cmd)) {
            cmd_help();
            continue;
        }
        
        // Look up command in map
        auto it = cmd_map.find(cmd);
        if (it != cmd_map.end()) {
            it->second->handler();
        } else {
            printf("Unknown command. Type 'help' for available commands.\n");
        }
    }
    
    return 0;
} 