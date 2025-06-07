#ifndef CLI_H
#define CLI_H

#include <string>

using namespace std;

class CommandLineInterface {
private:
    string _buffer;

public:
    CommandLineInterface();
    void handle_char(char c);
    static void print_prompt();
};

#endif //CLI_H
