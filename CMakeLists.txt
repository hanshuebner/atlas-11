cmake_minimum_required(VERSION 3.13)

# Set platform and board for PGA2350
set(PICO_PLATFORM "rp2350")
set(PICO_BOARD "pga2350")

# Add board search directories
list(APPEND PICO_BOARD_HEADER_DIRS "${PIMORONI_PICO_PATH}/pga2350")
list(APPEND PICO_BOARD_CMAKE_DIRS "${PIMORONI_PICO_PATH}/pga2350")

include(pico_sdk_import.cmake)
include(pimoroni_pico_import.cmake)

project(atlas-11 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(atlas-11
    main.c
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(atlas-11 1)
pico_enable_stdio_uart(atlas-11 0)

# Add the standard library to the build
target_link_libraries(atlas-11 pico_stdlib)

# Create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(atlas-11) 