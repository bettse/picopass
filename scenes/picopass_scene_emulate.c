#include "../picopass_i.h"
#include <dolphin/dolphin.h>
#include "../h10301.h"

#define TAG "PicopassSceneEmulate"

NfcCommand picopass_scene_listener_callback(PicopassListenerEvent event, void* context) {
    UNUSED(event);
    UNUSED(context);

    return NfcCommandContinue;
}

void picopass_scene_emulate_on_enter(void* context) {
    Picopass* picopass = context;
    PicopassDevice* dev = picopass->dev;
    PicopassDeviceData* dev_data = &dev->dev_data;
    PicopassPacs* pacs = &dev_data->pacs;

    dolphin_deed(DolphinDeedNfcEmulate);

    Widget* widget = picopass->widget;
    widget_reset(widget);
    widget_add_icon_element(widget, 0, 3, &I_RFIDDolphinSend_97x61);
    widget_add_string_element(widget, 92, 30, AlignCenter, AlignTop, FontPrimary, "Emulating");

    // Reload credential data
    picopass_device_parse_credential(dev_data->card_data, pacs);
    picopass_device_parse_wiegand(pacs);

    uint64_t credential = 0;
    memcpy(&credential, pacs->credential, sizeof(uint64_t));
    credential = __builtin_bswap64(credential);

    uint32_t Bot = credential & 0xFFFFFFFF;
    uint32_t Mid = (credential >> 32) & 0xFFFFFFFF;
    uint32_t Top = 0; // H10301 only uses up to 64 bits
    wiegand_message_t packed = initialize_message_object(Top, Mid, Bot, pacs->bitLength);
    wiegand_card_t card;

    if (Unpack_H10301(&packed, &card)) {
      // Successfully unpacked H10301 card

      FuriString* desc = furi_string_alloc();
      furi_string_printf(
          desc,
          "FC:%lu CN:%llu",
          card.FacilityCode,
          card.CardNumber);

        widget_add_string_element(widget, 92, 40, AlignCenter, AlignTop, FontPrimary, furi_string_get_cstr(desc));
      furi_string_free(desc);
    } else {
        widget_add_string_element(widget, 92, 40, AlignCenter, AlignTop, FontPrimary, "PicoPass");
    }

    FURI_LOG_D(TAG, "Unpacked H10301: FC:%lu CN:%llu ParityValid: %u", card.FacilityCode, card.CardNumber, card.ParityValid);

    widget_add_string_element(
        widget, 34, 55, AlignLeft, AlignTop, FontSecondary, "Touch flipper to reader");

    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewWidget);
    picopass_blink_emulate_start(picopass);

    picopass->listener = picopass_listener_alloc(picopass->nfc, dev_data);
    picopass_listener_start(picopass->listener, picopass_scene_listener_callback, picopass);
}

bool picopass_scene_emulate_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == PicopassCustomEventWorkerExit) {
            consumed = true;
        } else if(event.event == PicopassCustomEventNrMacSaved) {
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneNrMacSaved);
            consumed = true;
        } else if (event.event == GuiButtonTypeRight) {
            consumed = true;
        } else if (event.event == GuiButtonTypeLeft) {
            consumed = true;
        } else if (event.event == GuiButtonTypeCenter) {
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_previous_scene(picopass->scene_manager);
    }
    return consumed;
}

void picopass_scene_emulate_on_exit(void* context) {
    Picopass* picopass = context;

    picopass_blink_stop(picopass);
    picopass_listener_stop(picopass->listener);
    picopass_listener_free(picopass->listener);

    // Clear view
    widget_reset(picopass->widget);
}
