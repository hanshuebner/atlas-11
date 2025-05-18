#include <cstdio>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "console.hpp"

namespace {
    uart_inst_t* const UART_ID = uart0;
    constexpr uint BAUD_RATE = 38400;
    constexpr uint UART_TX_PIN = 0;
    constexpr uint UART_RX_PIN = 1;

    // Initialize UART for console mode
    void init_uart() {
        uart_init(UART_ID, BAUD_RATE);
        gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
        gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    }
}

// Run console mode - returns when user presses Ctrl-"\"
void console_mode() {
    printf("Entering console mode (Ctrl-\\ to exit)...\n");
    init_uart();
    
    while (true) {
        // Check for input from USB
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == 0x1C) {  // Ctrl-\ (ASCII 28)
                break;
            }
            uart_putc(UART_ID, c);
        }
        
        // Check for input from UART
        if (uart_is_readable(UART_ID)) {
            int uart_c = uart_getc(UART_ID);
            putchar(uart_c);
        }
    }
    
    printf("\nExiting console mode...\n");
} 