#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "dcj11_gpio.h"
#include "bus_interface.h"
#include "fs.h"

// Command handler function type with parameters
using cmd_handler_t = void(*)(const vector<string>&);

// Command structure
struct cmd_entry {
    string name;
    cmd_handler_t handler;
    string usage;
};

class CommandLineInterface {
private:
    // Command table
    static const vector<cmd_entry> _cmd_table;

    // Command lookup table, contains abbreviations and full commands
    static unordered_map<string, const cmd_entry*> _cmd_map;

    static void init_cmd_map();

    string _buffer;
public:
    CommandLineInterface() {
        init_cmd_map();

        printf("Atlas-11 Firmware Starting...\n");
        cmd_help({});
        print_prompt();
    }
    static void print_prompt();
    void handle_char(char c);
    static void handle_command(const string& cmd);
    static void cmd_help(const vector<string>& args);
};

void CommandLineInterface::cmd_help(const vector<string>& args) {
    cout << endl
            << "ATLAS-11 Control Interface" << endl << endl
            << "Commands:" << endl;

    // Print help using the command table
    for (const auto& cmd : _cmd_table) {
        cout << setw(8) << setfill(' ') << cmd.name << " - " << cmd.usage << endl;
    }
}

// Command handlers
static void cmd_on(const vector<string>& args) {
    gpio_put(DCJ11_POWER_CTL, false);
    cout << "DJC11 SBC powered on" << endl;
}

static void cmd_off(const vector<string>& args) {
    gpio_put(DCJ11_POWER_CTL, true);
    cout << "DJC11 SBC powered off" << endl;
}

static void cmd_halt(const vector<string>& args) {
    gpio_put(DCJ11_HALT, true);
    sleep_ms(10);  // Hold HALT high for 10ms
    gpio_put(DCJ11_HALT, false);
    cout << "HALT signal asserted for 10ms" << endl;
}

static void cmd_update(const vector<string>& args) {
    cout << "Rebooting into firmware update mode..." << endl;
    sleep_ms(100);  // Give time for message to be printed
    reset_usb_boot(0, 0);  // Reboot into USB bootloader
}

static void console_mode(const vector<string>& args) {
    cmd_on(args);
    console_mode();
}


// Split string into vector of strings
static vector<string> split_string(const string& str) {
    vector<string> result;
    istringstream iss(str);
    string token;

    while (iss >> token) {
        result.push_back(token);
    }

    return result;
}

const vector<cmd_entry> CommandLineInterface::_cmd_table = {
    {"iosnoop", cmd_iosnoop, "Monitor I/O access on bus (e.g. 'iosnoop 20' for 20 captures, default 10)"},
    {"console", console_mode, "Enter UART console mode (38400 8N1) (powers on, too)"},
    {"halt", cmd_halt, "Assert HALT signal for 10ms"},
    {"ls", cmd_ls, "List files on SD card"},
    {"on", cmd_on, "Power on the DJC11 SBC"},
    {"off", cmd_off, "Power off the DJC11 SBC"},
    {"update", cmd_update, "Reboot into firmware update mode"},
    {"help", cmd_help, "Show help message"}
};

unordered_map<string, const cmd_entry*> CommandLineInterface::_cmd_map;

// Initialize command map with abbreviations
void
CommandLineInterface::init_cmd_map() {
    // First add all full command names
    for (const auto& cmd : _cmd_table) {
        _cmd_map[cmd.name] = &cmd;
    }

    // Track non-unique prefixes
    vector<string> non_unique;

    // Try all possible prefixes for each command
    for (const auto& cmd : _cmd_table) {
        for (size_t i = 1; i < cmd.name.length(); i++) {
            string prefix = cmd.name.substr(0, i);

            // Check if this prefix is already in the map
            if (_cmd_map.contains(prefix)) {
                non_unique.push_back(prefix);
            } else {
                _cmd_map[prefix] = &cmd;
            }
        }
    }

    // Remove all non-unique prefixes
    for (const auto& prefix : non_unique) {
        _cmd_map.erase(prefix);
    }
}

void CommandLineInterface::print_prompt() {
    cout << "> ";
    cout.flush();
}

void CommandLineInterface::handle_char(char c) {
    switch (c) {
        case '\r':
        case '\n':
            cout << endl;
            handle_command(_buffer);
            _buffer.clear();
            print_prompt();
            break;
        case '\b':
        case '\177':
            if (!_buffer.empty()) {
                _buffer.pop_back();
                cout << "\b \b";
            }
            break;
        default:
            if (c >= 32 && c <= 126) {  // Printable ASCII
                _buffer.push_back(c);
                cout.put(c);
            }
    }
    cout.flush();
}

void
CommandLineInterface::handle_command(const string& cmd) {
    // Split command into words
    vector<string> args = split_string(cmd);
    if (args.empty()) {
        return;
    }

    // Look up command in map
    auto it = _cmd_map.find(args[0]);
    if (it != _cmd_map.end()) {
        it->second->handler(args);
    } else {
        printf("Unknown command. Type 'help' for available commands.\n");
    }
}

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

int main() {
    // Initialize stdio
    stdio_uart_init();

    init_gpio();

    CommandLineInterface cli;

    while (true) {
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            cli.handle_char(c);
        }
    }

    return 0;
}
