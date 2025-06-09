#ifndef FS_H
#define FS_H

#include <string>
#include <iostream>

using namespace std;

// Read entire file contents into a string
string slurp(const string& filename);

// Generic function to safely mount SD card, execute action, and unmount
template<typename F, typename T = void> T with_sdcard_mounted(F action, T default_value = T());

#endif // FS_H