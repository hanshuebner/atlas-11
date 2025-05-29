
#ifndef TERMINAL_H
#define TERMINAL_H

#include <string>

using std::string;

class DL11;

void terminal_session(const string& port_name, DL11* dl11);

#endif //TERMINAL_H
