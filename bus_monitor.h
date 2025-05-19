#ifndef BUS_MONITOR_H
#define BUS_MONITOR_H

#include <string>
#include <vector>

// Bus monitoring function
void __not_in_flash_func(cmd_iosnoop)(const std::vector<std::string>& args);

#endif // BUS_MONITOR_H