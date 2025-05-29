#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include "pico/stdlib.h"
#include "tf_card.h"
#include "ff.h"
#include "dcj11_gpio.h"
#include "fs.h"

using namespace std;

pico_fatfs_spi_config_t fatfs_config = {
    spi1,  // if unmatched SPI pin assignments with spi0/spi1 or explicitly designated as NULL, SPI PIO will be configured
    CLK_SLOW_DEFAULT,
    CLK_FAST_DEFAULT,
    DCJ11_SDCARD_MISO,
    DCJ11_SDCARD_CS,
    DCJ11_SDCARD_SCK,
    DCJ11_SDCARD_MOSI,
    true
};

// Helper function to convert FRESULT to error string
static const char* f_result_to_str(FRESULT res) {
    switch (res) {
        case FR_OK: return "Succeeded";
        case FR_DISK_ERR: return "A hard error occurred in the low level disk I/O layer";
        case FR_INT_ERR: return "Assertion failed";
        case FR_NOT_READY: return "The physical drive cannot work";
        case FR_NO_FILE: return "Could not find the file";
        case FR_NO_PATH: return "Could not find the path";
        case FR_INVALID_NAME: return "The path name format is invalid";
        case FR_DENIED: return "Access denied due to prohibited access or directory full";
        case FR_EXIST: return "Access denied due to prohibited access";
        case FR_INVALID_OBJECT: return "The file/directory object is invalid";
        case FR_WRITE_PROTECTED: return "The physical drive is write protected";
        case FR_INVALID_DRIVE: return "The logical drive number is invalid";
        case FR_NOT_ENABLED: return "The volume has no work area";
        case FR_NO_FILESYSTEM: return "There is no valid FAT volume";
        case FR_MKFS_ABORTED: return "The f_mkfs() aborted due to any problem";
        case FR_TIMEOUT: return "Could not get a grant to access the volume within defined period";
        case FR_LOCKED: return "The operation is rejected according to the file sharing policy";
        case FR_NOT_ENOUGH_CORE: return "LFN working buffer could not be allocated";
        case FR_TOO_MANY_OPEN_FILES: return "Number of open files > FF_FS_LOCK";
        case FR_INVALID_PARAMETER: return "Given parameter is invalid";
        default: return "Unknown error";
    }
}

// Helper function to format file size in human readable format
static string format_size(uint32_t size) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit = 0;
    float value = size;

    while (value >= 1024 && unit < 3) {
        value /= 1024;
        unit++;
    }

    ostringstream oss;
    if (unit == 0) {
        oss << static_cast<int>(value) << " " << units[unit];
    } else {
        oss << fixed << setprecision(1) << value << " " << units[unit];
    }
    return oss.str();
}

// Helper function to format file attributes
static string format_attr(uint8_t attr) {
    const char* const attr_chars = "drhsa";  // directory, read-only, hidden, system, archive
    const uint8_t attr_bits[] = {AM_DIR, AM_RDO, AM_HID, AM_SYS, AM_ARC};
    ostringstream result;

    for (const auto& bit : attr_bits) {
        result << ((attr & bit) ? attr_chars[&bit - attr_bits] : '-');
    }
    return result.str();
}

static void init_sdcard() {
    pico_fatfs_set_config(&fatfs_config);
}

void cmd_ls(const vector<string>& args) {
    FATFS fs;
    FRESULT res;

    init_sdcard();

    // Try to mount the filesystem
    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        cout << "Failed to mount SD card: " << f_result_to_str(res) << endl;
        return;
    }

    DIR dir;
    FILINFO fno;

    // Open the root directory
    res = f_opendir(&dir, "");
    if (res != FR_OK) {
        cout << "Failed to open directory: " << f_result_to_str(res) << endl;
        f_mount(nullptr, "", 0);
        return;
    }

    // Read and print directory entries
    while (true) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0) break;

        // Skip . and .. entries
        if (fno.fname[0] == '.') continue;

        // Print file info in ls -l format
        cout << format_attr(fno.fattrib) << " "
             << setw(10) << format_size(fno.fsize) << " "
             << fno.fname << endl;
    }

    // Close directory and unmount
    f_closedir(&dir);
    f_mount(nullptr, "", 0);
}
