#include <cstdio>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "console.hpp"
#include "dcj11_gpio.h"

namespace {
    uart_inst_t* const UART_ID = uart0;
    constexpr uint BAUD_RATE = 38400;
    constexpr uint64_t ESCAPE_DELAY_US = 200000; // 200ms in microseconds

    // Initialize UART for console mode
    void init_uart() {
        uart_init(UART_ID, BAUD_RATE);
        gpio_set_function(DCJ11_UART0_TX, GPIO_FUNC_UART);
        gpio_set_function(DCJ11_UART0_RX, GPIO_FUNC_UART);
    }
}

// Run console mode - returns when user presses Ctrl-"\"
void console_mode() {
    printf("Entering console mode (Ctrl-\\ to exit)...\n");
    init_uart();

    uint64_t last_input_time = 0;

    while (true) {
        // Check for input from USB
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            uint64_t current_time = time_us_64();

            if (c == 0x1C) {  // Ctrl-\ (ASCII 28)
                // Only exit if no input in last 200ms
                if (current_time - last_input_time >= ESCAPE_DELAY_US) {
                    break;
                }
            }

            uart_putc(UART_ID, c);
            last_input_time = current_time;
        }

        // Check for input from UART
        if (uart_is_readable(UART_ID)) {
            int uart_c = uart_getc(UART_ID);
            putchar(uart_c);
        }
    }

    printf("\nExiting console mode...\n");
}
