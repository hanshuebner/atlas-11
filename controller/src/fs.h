#ifndef FS_H
#define FS_H

#include <string>
#include <vector>
#include <functional>

// List files on SD card
void cmd_ls(const std::vector<std::string>& args);

// Read entire file contents into a string
std::string slurp(const std::string& filename);

// Generic function to safely mount SD card, execute action, and unmount
template<typename F, typename T = void>
T with_sdcard_mounted(F action, T default_value = T());

#endif // FS_H