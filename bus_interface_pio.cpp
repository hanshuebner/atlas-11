#include "bus_interface_pio.h"
#include "bus_interface.pio.h"

// PIO program configuration
static const uint DAL_PIN_MASK = ((1 << DAL_COUNT) - 1) << DAL_BASE;
static const uint nALE_PIN_MASK = 1 << nALE_PIN;
static const uint nSCTL_PIN_MASK = 1 << nSCTL_PIN;
static const uint nCONT_PIN_MASK = 1 << nCONT_PIN;

void bus_interface_pio_init(void) {
    // Load the PIO program
    uint offset = pio_add_program(BUS_INTERFACE_PIO, &bus_interface_program);
    
    // Configure the state machine
    pio_sm_config c = bus_interface_program_get_default_config(offset);
    
    // Set the pins
    sm_config_set_in_pins(&c, DAL_BASE);  // Input pins start at DAL0
    sm_config_set_out_pins(&c, DAL_BASE, DAL_COUNT);  // Output pins are DAL0-15
    sm_config_set_set_pins(&c, DAL_BASE, DAL_COUNT);  // Set pins are DAL0-15
    
    // Configure the side-set pins
    sm_config_set_sideset_pins(&c, nCONT_PIN);
    
    // Configure the clock divider
    float div = clock_get_hz(clk_sys) / (1000000);  // 1MHz
    sm_config_set_clkdiv(&c, div);
    
    // Configure the shift registers
    sm_config_set_in_shift(&c, false, true, 32);  // Shift right, autopush
    sm_config_set_out_shift(&c, false, true, 32); // Shift right, autopull
    
    // Initialize the state machine
    pio_sm_init(BUS_INTERFACE_PIO, BUS_INTERFACE_SM, offset, &c);
    
    // Enable the state machine
    pio_sm_set_enabled(BUS_INTERFACE_PIO, BUS_INTERFACE_SM, true);
}

void bus_interface_pio_deinit(void) {
    // Disable the state machine
    pio_sm_set_enabled(BUS_INTERFACE_PIO, BUS_INTERFACE_SM, false);
    
    // Remove the program
    pio_remove_program(BUS_INTERFACE_PIO, &bus_interface_program, 0);
}

void bus_interface_write(uint16_t address, uint16_t data) {
    // The PIO program handles the write operation automatically
    // We just need to ensure the data is in the TX FIFO
    pio_sm_put(BUS_INTERFACE_PIO, BUS_INTERFACE_SM, data);
}

uint16_t bus_interface_read(uint16_t address) {
    // The PIO program handles the read operation automatically
    // We just need to read from the RX FIFO
    return pio_sm_get(BUS_INTERFACE_PIO, BUS_INTERFACE_SM);
} 