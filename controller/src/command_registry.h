#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <iostream>

using namespace std;

struct Command {
    string name;
    string help;
    function<void(ostream&, const vector<string>&)> handler;

    // Default constructor
    Command() = default;

    // Main constructor
    Command(string name, string help,
            function<void(ostream&, const vector<string>&)> handler);
};

class CommandRegistry {
private:
    class Impl;
    unique_ptr<Impl> pimpl;

    CommandRegistry();

    void init_cmd_map() const;

public:
    CommandRegistry(const CommandRegistry&) = delete;
    CommandRegistry& operator=(const CommandRegistry&) = delete;

    ~CommandRegistry();
    static CommandRegistry& instance();
    void register_command(const Command& cmd);
    [[nodiscard]] const Command* find_command(const string& name) const;
    [[nodiscard]] vector<string> get_command_names() const;
};