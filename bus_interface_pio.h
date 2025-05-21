#ifndef BUS_INTERFACE_PIO_H
#define BUS_INTERFACE_PIO_H

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "dcj11_gpio.h"

// PIO program configuration
#define BUS_INTERFACE_PIO pio0
#define BUS_INTERFACE_SM 0
#define BUS_INTERFACE_IRQ PIO0_IRQ_0

// Function declarations
void bus_interface_pio_init(void);
void bus_interface_pio_deinit(void);

// Bus control functions
void bus_interface_write(uint16_t address, uint16_t data);
uint16_t bus_interface_read(uint16_t address);

#endif // BUS_INTERFACE_PIO_H 