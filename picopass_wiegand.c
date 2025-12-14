#include "picopass_wiegand.h"
#include <string.h>

static inline uint8_t oddparity32(uint32_t x) {
    return bit_lib_test_parity_32(x, BitLibParityOdd);
}

static inline uint8_t evenparity32(uint32_t x) {
    return bit_lib_test_parity_32(x, BitLibParityEven);
}

wiegand_message_t initialize_wiegand_message_object(uint32_t top, uint32_t mid, uint32_t bot, int n) {
    wiegand_message_t result;
    memset(&result, 0, sizeof(wiegand_message_t));

    result.Top = top;
    result.Mid = mid;
    result.Bot = bot;
    if(n > 0) result.Length = n;
    return result;
}

bool Pack_H10301(wiegand_card_t* card, wiegand_message_t* packed) {
    memset(packed, 0, sizeof(wiegand_message_t));

    packed->Length = 26; // Set number of bits
    packed->Bot |= (card->CardNumber & 0xFFFF) << 1;
    packed->Bot |= (card->FacilityCode & 0xFF) << 17;
    packed->Bot |= oddparity32((packed->Bot >> 1) & 0xFFF);
    packed->Bot |= (evenparity32((packed->Bot >> 13) & 0xFFF)) << 25;

    return true;
}

bool Unpack_H10301(wiegand_message_t* packed, wiegand_card_t* card) {
    if(packed->Length != 26) return false; // Wrong length? Stop here.

    memset(card, 0, sizeof(wiegand_card_t));

    card->CardNumber = (packed->Bot >> 1) & 0xFFFF;
    card->FacilityCode = (packed->Bot >> 17) & 0xFF;
    card->ParityValid = (oddparity32((packed->Bot >> 1) & 0xFFF) == (packed->Bot & 1)) &&
                        ((evenparity32((packed->Bot >> 13) & 0xFFF)) == ((packed->Bot >> 25) & 1));
    return card->ParityValid;
}
