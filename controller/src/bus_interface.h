#ifndef BUS_MONITOR_H
#define BUS_MONITOR_H

#include <string>
#include <vector>

using namespace std;

// Bus monitoring function
void __not_in_flash_func(cmd_iosnoop)(const vector<string>& args);

void console_mode();

#endif // BUS_MONITOR_H