#include "command_registry.h"
#include <unordered_map>

using namespace std;

class CommandRegistry::Impl {
public:
    unordered_map<string, Command> commands;
    mutable unordered_map<string, const Command*> cmd_map;
    mutable bool cmd_map_initialized = false;
};

CommandRegistry::CommandRegistry() : pimpl(make_unique<Impl>()) {}
CommandRegistry::~CommandRegistry() = default;

CommandRegistry& CommandRegistry::instance() {
    static CommandRegistry instance;
    return instance;
}

void CommandRegistry::register_command(const Command& cmd) {
    pimpl->commands[cmd.name] = cmd;
    pimpl->cmd_map_initialized = false;  // Reset initialization flag when new command is registered
}

void CommandRegistry::init_cmd_map() const {
    if (pimpl->cmd_map_initialized) {
        return;
    }

    // Clear existing map
    pimpl->cmd_map.clear();

    // First add all full command names
    for (const auto& [name, cmd] : pimpl->commands) {
        pimpl->cmd_map[name] = &cmd;
    }

    // Track non-unique prefixes
    vector<string> non_unique;

    // Try all possible prefixes for each command
    for (const auto& [name, cmd] : pimpl->commands) {
        for (size_t i = 1; i < name.length(); i++) {
            string prefix = name.substr(0, i);

            // Check if this prefix is already in the map
            if (pimpl->cmd_map.find(prefix) != pimpl->cmd_map.end()) {
                non_unique.push_back(prefix);
            } else {
                pimpl->cmd_map[prefix] = &cmd;
            }
        }
    }

    // Remove all non-unique prefixes
    for (const auto& prefix : non_unique) {
        pimpl->cmd_map.erase(prefix);
    }

    pimpl->cmd_map_initialized = true;
}

const Command* CommandRegistry::find_command(const string& name) const {
    if (!pimpl->cmd_map_initialized) {
        init_cmd_map();
    }
    
    auto it = pimpl->cmd_map.find(name);
    return it != pimpl->cmd_map.end() ? it->second : nullptr;
}

vector<string> CommandRegistry::get_command_names() const {
    vector<string> names;
    names.reserve(pimpl->commands.size());
    for (const auto& [name, _] : pimpl->commands) {
        names.push_back(name);
    }
    return names;
} 