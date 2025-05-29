#include <stdio.h>
#include <string>
#include "pico/stdlib.h"
#include "tf_card.h"
#include "ff.h"
#include "dcj11_gpio.h"
#include "fs.h"

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
static std::string format_size(uint32_t size) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit = 0;
    float value = size;

    while (value >= 1024 && unit < 3) {
        value /= 1024;
        unit++;
    }

    char buf[32];
    if (unit == 0) {
        snprintf(buf, sizeof(buf), "%d %s", (int)value, units[unit]);
    } else {
        snprintf(buf, sizeof(buf), "%.1f %s", value, units[unit]);
    }
    return std::string(buf);
}

// Helper function to format file attributes
static std::string format_attr(uint8_t attr) {
    std::string result;
    if (attr & AM_DIR) result += "d";
    else result += "-";
    if (attr & AM_RDO) result += "r";
    else result += "-";
    if (attr & AM_HID) result += "h";
    else result += "-";
    if (attr & AM_SYS) result += "s";
    else result += "-";
    if (attr & AM_ARC) result += "a";
    else result += "-";
    return result;
}

static void init_sdcard() {
    bool spi_configured = pico_fatfs_set_config(&fatfs_config);
    if (spi_configured) {
        printf("SPI configured for SD card\n");
    } else {
        printf("SPI PIO configured for SD card\n");
    }
}

void cmd_ls(const std::vector<std::string>& args) {
    FATFS fs;
    FRESULT res;

    init_sdcard();

    // Try to mount the filesystem
    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        printf("Failed to mount SD card: %s\n", f_result_to_str(res));
        return;
    }

    DIR dir;
    FILINFO fno;

    // Open the root directory
    res = f_opendir(&dir, "");
    if (res != FR_OK) {
        printf("Failed to open directory: %s\n", f_result_to_str(res));
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
        printf("%s %10s %s\n",
               format_attr(fno.fattrib).c_str(),
               format_size(fno.fsize).c_str(),
               fno.fname);
    }

    // Close directory and unmount
    f_closedir(&dir);
    f_mount(nullptr, "", 0);
}