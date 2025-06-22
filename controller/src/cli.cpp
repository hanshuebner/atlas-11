#include "cli.h"
#include "command_system.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <pico/stdlib.h>
#include <pico/bootrom.h>
#include "dcj11_gpio.h"
#include "bus_interface.h"
#include "fs.h"
#include "terminal_session.h"
#include "hardware/watchdog.h"

using namespace std;

// Register all commands
vector<Command> commands = {
    {
        "halt", "Assert HALT signal for 10ms",
        [](ostream &out, const vector<string> &args) {
            gpio_put(DCJ11_HALT, true);
            sleep_ms(10); // Hold HALT high for 10ms
            gpio_put(DCJ11_HALT, false);
            out << "HALT signal asserted for 10ms" << endl;
        }
    },
    {
        "on", "Power on the DJC11 SBC",
        [](ostream &out, const vector<string> &args) {
            gpio_put(DCJ11_POWER_CTL, true);
            out << "DJC11 SBC powered on" << endl;
        }
    },
    {
        "off", "Power off the DJC11 SBC",
        [](ostream &out, const vector<string> &args) {
            gpio_put(DCJ11_POWER_CTL, false);
            out << "DJC11 SBC powered off" << endl;
        }
    },
    {
        "update", "Reboot into firmware update mode",
        [](ostream &out, const vector<string> &args) {
            out << "Rebooting into firmware update mode..." << endl;
            sleep_ms(100); // Give time for message to be printed
            reset_usb_boot(0, 0); // Reboot into USB bootloader
        }
    },
    {
        "reset", "Reset the RP2350 Microcontroller",
        [](ostream &out, const vector<string> &args) {
            watchdog_enable(1, 1); // timeout = 1 us
            while (1); // wait for watchdog to fire
        }
    },
    {
        "help", "Show help message",
        [](ostream &out, const vector<string> &args) {
            out << endl
                    << "ATLAS-11 Control Interface" << endl << endl
                    << "Commands:" << endl;

            // Print help using the command registry
            for (const auto &name: CommandRegistry::instance().get_command_names()) {
                const Command *cmd = CommandRegistry::instance().find_command(name);
                if (cmd) {
                    out << setw(8) << setfill(' ') << cmd->name << " - " << cmd->help << endl;
                }
            }
        }
    },
};

CommandLineInterface::CommandLineInterface() {
    printf("Atlas-11 Firmware Starting...\n");
    CommandRegistry::instance().find_command("help")->handler(cout, {});
    print_prompt();
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
            CommandInterpreter::execute(_buffer, cout);
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
            if (c >= 32 && c <= 126) {
                // Printable ASCII
                _buffer.push_back(c);
                cout.put(c);
            }
    }
    cout.flush();
}
