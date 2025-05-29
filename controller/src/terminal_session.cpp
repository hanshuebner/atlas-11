
#include <cstdio>
#include <iostream>

#include "dl11.h"
#include "hardware/timer.h"
#include "pico/error.h"
#include "pico/multicore.h"
#include "pico/stdio.h"
#include "pico/util/queue.h"

#include "terminal_session.h"

using namespace std;

constexpr uint8_t DISCONNECT_CHAR = 0x1c; // ^\

void terminal_session(const string& port_name, DL11* dl11) {
    cout << "Connecting to " << port_name << ", press Ctrl-" << static_cast<char>(DISCONNECT_CHAR + '@') << " to disconnect..." << endl;

    bool done = false;
    uint64_t last_input = time_us_64();
    while (!done) {
        if (!queue_is_empty(dl11->get_send_queue())) {
            uint8_t value;
            queue_remove_blocking(dl11->get_send_queue(), &value);
            putchar(value);
            fflush(stdout);
        }
        int value = getchar_timeout_us(0);
        switch (value) {
            case PICO_ERROR_TIMEOUT:
                break;
            case DISCONNECT_CHAR:
                if (last_input + 500000 < time_us_64()) {
                    done = true;
                    break;
                }
                // fall through
            default:
                last_input = time_us_64();
                queue_try_add(dl11->get_receive_queue(), &value);
        }
    }

    cout << endl << "Disconnected." << endl;
}

