#pragma once

#include "command_registry.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

class CommandInterpreter {
private:
    [[nodiscard]] static vector<string> split_command(const string& input);

public:
    static void execute(const string& input, ostream& out);
};