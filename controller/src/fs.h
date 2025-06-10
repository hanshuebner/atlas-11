#ifndef FS_H
#define FS_H

#include <string>
#include <iostream>
#include "ff.h"

using namespace std;

// Read entire file contents into a string
string slurp(const string& filename);

void init_sdcard();
const char *f_result_to_str(uint16_t res);

enum {
    FR_EXT_BLOCK_WRITE_FAILED = 100,
    FR_EXT_BLOCK_READ_FAILED = 101,
};

// RAII wrapper for SD card mounting
class ScopedSDCardMount {
    FATFS &fs;
    bool mounted;

public:
    explicit ScopedSDCardMount(FATFS &fs) : fs(fs), mounted(false) {
        init_sdcard();
        FRESULT res = f_mount(&fs, "", 1);
        if (res != FR_OK) {
            cout << "Failed to mount SD card: " << f_result_to_str(res) << endl;
            return;
        }
        mounted = true;
    }

    ~ScopedSDCardMount() {
        if (mounted) {
            f_mount(nullptr, "", 0);
        }
    }

    [[nodiscard]] bool is_mounted() const { return mounted; }
};

// Generic function to safely mount SD card, execute action, and unmount
template<typename F, typename T>
T with_sdcard_mounted(F action, T default_value) {
    FATFS fs;
    ScopedSDCardMount mount(fs);

    if (!mount.is_mounted()) {
        return default_value;
    }

    return action();
}
template<typename F>
void with_sdcard_mounted(F action) {
    FATFS fs;
    ScopedSDCardMount mount(fs);

    if (!mount.is_mounted()) {
        return;
    }

    action();
}

#endif // FS_H