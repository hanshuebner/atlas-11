
#ifndef CLI_H
#define CLI_H

#include <string>
#include <unordered_map>
#include <vector>

#include "main.h"

using namespace std;

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
    explicit CommandLineInterface() {
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


#endif //CLI_H
