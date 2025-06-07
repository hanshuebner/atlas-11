#include "command_system.h"
#include <sstream>

using namespace std;

Command::Command(string name, string help,
                 function<void(ostream &, const vector<string> &)> handler)
    : name(std::move(name)),
      help(std::move(help)),
      handler(std::move(handler)) {
    CommandRegistry::instance().register_command(*this);
}

vector<string> CommandInterpreter::split_command(const string& input) {
    vector<string> result;
    string current;
    bool in_quotes = false;

    for (char c : input) {
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (c == ' ' && !in_quotes) {
            if (!current.empty()) {
                result.push_back(std::move(current));
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        result.push_back(std::move(current));
    }

    return result;
}

void CommandInterpreter::execute(const string& input, ostream& out) {
    auto parts = split_command(input);
    if (parts.empty()) {
        return;
    }

    const string& cmd_name = parts[0];
    vector<string> args(parts.begin() + 1, parts.end());

    const Command* cmd = CommandRegistry::instance().find_command(cmd_name);
    if (!cmd) {
        out << "Unknown command: " << cmd_name << endl;
        return;
    }

    try {
        cmd->handler(out, args);
    } catch (const exception& e) {
        out << "Error executing command: " << e.what() << endl;
    }
}
