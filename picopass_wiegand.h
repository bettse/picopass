#pragma once

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <lib/bit_lib/bit_lib.h>

// Structure for packed wiegand messages
// Always align lowest value (last transmitted) bit to ordinal position 0 (lowest valued bit bottom)
typedef struct {
    uint8_t Length; // Number of encoded bits in wiegand message (excluding headers and preamble)
    uint32_t Top; // Bits in x<<64 positions
    uint32_t Mid; // Bits in x<<32 positions
    uint32_t Bot; // Lowest ordinal positions
} wiegand_message_t;

// Structure for unpacked wiegand card, like HID prox
typedef struct {
    uint32_t FacilityCode;
    uint64_t CardNumber;
    uint32_t IssueLevel;
    uint32_t OEM;
    bool ParityValid; // Only valid for responses
} wiegand_card_t;

wiegand_message_t
    initialize_wiegand_message_object(uint32_t top, uint32_t mid, uint32_t bot, int n);

bool Pack_H10301(wiegand_card_t* card, wiegand_message_t* packed);
bool Unpack_H10301(wiegand_message_t* packed, wiegand_card_t* card);
