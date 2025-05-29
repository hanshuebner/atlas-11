/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string>
#include <tusb.h>
#include <bsp/board_api.h>

using namespace std;

// set some example Vendor and Product ID
// the board will use to identify at the host
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define CDC_EXAMPLE_VID     0xCafe
// use _PID_MAP to generate unique PID for each interface
#define CDC_EXAMPLE_PID     (0x4000 | _PID_MAP(CDC, 0))
// set USB 2.0
#define CDC_EXAMPLE_BCD     0x0200

// defines a descriptor that will be communicated to the host
tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = CDC_EXAMPLE_BCD,

    .bDeviceClass = TUSB_CLASS_MISC, // CDC is a subclass of misc
    .bDeviceSubClass = MISC_SUBCLASS_COMMON, // CDC uses common subclass
    .bDeviceProtocol = MISC_PROTOCOL_IAD, // CDC uses IAD

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE, // 64 bytes

    .idVendor = CDC_EXAMPLE_VID,
    .idProduct = CDC_EXAMPLE_PID,
    .bcdDevice = 0x0100, // Device release number

    .iManufacturer = 0x01, // Index of manufacturer string
    .iProduct = 0x02, // Index of product string
    .iSerialNumber = 0x03, // Index of serial number string

    .bNumConfigurations = 0x01 // 1 configuration
};

enum {
    ITF_NUM_CDC_0 = 0,
    ITF_NUM_CDC_0_DATA,
    ITF_NUM_CDC_1,
    ITF_NUM_CDC_1_DATA,
    ITF_NUM_CDC_2,
    ITF_NUM_CDC_2_DATA,
    ITF_NUM_CDC_3,
    ITF_NUM_CDC_3_DATA,
    ITF_NUM_CDC_4,
    ITF_NUM_CDC_4_DATA,
    ITF_NUM_TOTAL
};

// total length of configuration descriptor
#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + CFG_TUD_CDC * TUD_CDC_DESC_LEN)

// configure descriptor (for 9 CDC interfaces)
uint8_t const desc_configuration[] = {
    // config descriptor | how much power in mA, count of interfaces, ...
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x80, 100),

    // CDC Communication Interfaces
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 4, 0x8A, 8, 0x01, 0x81, 64),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_1, 4, 0x8B, 8, 0x02, 0x82, 64),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_2, 4, 0x8C, 8, 0x03, 0x83, 64),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_3, 4, 0x8D, 8, 0x04, 0x84, 64),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_4, 4, 0x8E, 8, 0x05, 0x85, 64),
};

// more device descriptor this time the qualifier
tusb_desc_device_qualifier_t const desc_device_qualifier = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = CDC_EXAMPLE_BCD,

    .bDeviceClass = TUSB_CLASS_CDC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .bNumConfigurations = 0x01,
    .bReserved = 0x00
};

// String descriptors referenced with .i... in the descriptor tables

enum {
    STRID_LANGID = 0,   // 0: supported language ID
    STRID_MANUFACTURER, // 1: Manufacturer
    STRID_PRODUCT,      // 2: Product
    STRID_SERIAL,       // 3: Serials
    STRID_CDC_0,        // 4: CDC Interface 0
    STRID_CDC_1,        // 5: CDC Interface 1
    STRID_CDC_2,        // 6: CDC Interface 2
    STRID_CDC_3,        // 7: CDC Interface 3
};

// array of pointer to descriptor strings
char16_t const *string_desc_arr[] = {
    nullptr,                        // 0: Language ID (see below)
    u"Hans Hübner",                 // 1: Manufacturer
    u"ATLAS-11",                    // 2: Product
    nullptr,                        // 3: Serials (null so it uses unique ID if available)
    u"Console",                     // 4: CDC Interface 0
    u"TX0",                         // 5: CDC Interface 1,
    u"TX1",                         // 6: CDC Interface 2,
    u"TX2",                         // 7: CDC Interface 3,
    u"TX3",                         // 8: CDC Interface 4,
};

// buffer to hold the string descriptor during the request | plus 1 for the null terminator
static uint16_t desc_str[32 + 1];

// --------------------------------------------------------------------+
// IMPLEMENTATION
// --------------------------------------------------------------------+

extern "C" uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}

extern "C" uint8_t const* tud_descriptor_device_qualifier_cb(void)
{
    return (uint8_t const *)&desc_device_qualifier;
}

extern "C" uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
    // avoid unused parameter warning and keep function signature consistent
    (void)index;

    return desc_configuration;
}

extern "C" uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    // TODO: check lang id
    (void) langid;
    size_t char_count;

    // Determine which string descriptor to return
    switch (index) {
        case STRID_LANGID:
            memcpy(&desc_str[1], (const char[]) { 0x09, 0x04 }, 2);
            char_count = 1;
            break;

        case STRID_SERIAL:
            // try to read the serial from the board
            char_count = board_usb_get_serial(desc_str + 1, 32);
            break;

        default:
            // COPYRIGHT NOTE: Based on TinyUSB example
            // Windows wants utf16le

            // Determine which string descriptor to return
            if ( !(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) ) {
                return nullptr;
            }

            // Copy string descriptor into _desc_str
            const char16_t *str = string_desc_arr[index];

            char_count = char_traits<char16_t>::length(str);
            size_t const max_count = sizeof(desc_str) / sizeof(desc_str[0]) - 1; // -1 for string type
            // Cap at max char
            if (char_count > max_count) {
                char_count = max_count;
            }

            // Copy string to descriptor
            for (size_t i = 0; i < char_count; i++) {
                desc_str[1 + i] = str[i];
            }
            break;
    }

    // First byte is the length (including header), second byte is string type
    desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (char_count * 2 + 2));

    return desc_str;
}
