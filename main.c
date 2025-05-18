#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "console.h"

#define GPIO_PIN 2
#define MAX_CMD_LEN 32

void print_help(void) {
    printf("\nAtlas-11 GPIO Control Interface\n");
    printf("Commands:\n");
    printf("  on  - Power on the DJC11 SBC\n");
    printf("  off - Power off the DJC11 SBC\n");
    printf("  help - Show this help message\n");
    printf("  update - Reboot into firmware update mode\n");
    printf("  console - Enter UART console mode (38400 8N1)\n");
}

// Read a line of input with basic line editing capabilities
// Returns 1 if a command was entered, 0 if empty line
int read_line(char *buffer, int max_len) {
    int c;
    int pos = 0;
    
    printf("\n> ");
    
    while (pos < max_len - 1) {
        c = getchar();
        
        // Handle enter key
        if (c == '\r' || c == '\n') {
            printf("\n");
            if (pos == 0) {
                return 0;  // Empty line
            }
            buffer[pos] = '\0';
            return 1;  // Command entered
        }
        
        // Handle backspace
        if (c == '\b' || c == 127) {
            if (pos > 0) {
                pos--;
                printf("\b \b");  // Move back, print space, move back again
            }
            continue;
        }
        
        // Handle regular character
        if (c >= 32 && c <= 126) {  // Printable ASCII
            buffer[pos++] = c;
            putchar(c);
        }
    }
    
    buffer[pos] = '\0';
    return 1;  // Buffer full, treat as command
}

int main() {
    // Initialize stdio
    stdio_init_all();
    
    // Initialize GPIO
    gpio_init(GPIO_PIN);
    gpio_set_dir(GPIO_PIN, GPIO_OUT);
    gpio_put(GPIO_PIN, 0);  // Start with pin low
    
    // Wait for USB CDC to be ready
    sleep_ms(1000);
    
    printf("Atlas-11 Firmware Starting...\n");
    print_help();
    
    char cmd[MAX_CMD_LEN];
    
    while (1) {
        if (!read_line(cmd, MAX_CMD_LEN)) {
            print_help();
            continue;
        }
        
        // Process command
        if (strcmp(cmd, "on") == 0) {
            gpio_put(GPIO_PIN, 1);
            printf("DJC11 SBC powered on\n");
        }
        else if (strcmp(cmd, "off") == 0) {
            gpio_put(GPIO_PIN, 0);
            printf("DJC11 SBC powered off\n");
        }
        else if (strcmp(cmd, "help") == 0) {
            print_help();
        }
        else if (strcmp(cmd, "update") == 0) {
            printf("Rebooting into firmware update mode...\n");
            sleep_ms(100);  // Give time for message to be printed
            reset_usb_boot(0, 0);  // Reboot into USB bootloader
        }
        else if (strcmp(cmd, "console") == 0) {
            console_mode();
        }
        else {
            printf("Unknown command. Type 'help' for available commands.\n");
        }
    }
    
    return 0;
} 