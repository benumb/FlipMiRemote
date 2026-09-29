/*
 * FlipMiRemote - BLE remote compatibility tester for Xiaomi Mi Box / Android TV
 *
 * V0.5: Xiaomi-name BLE discovery probes.
 *
 * GPL-2.0
 */

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_bt.h>
#include <bt/bt_service/bt.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <input/input.h>
#include <storage/storage.h>

#include "flipmi_ble_profile.h"

#define TAG "FlipMiRemote"
#define HID_BT_KEYS_STORAGE_NAME ".bt_hid.keys"
#define MODE_COUNT 7

typedef struct {
    const char* label;
    FlipMiBleProfileParams profile;
} FlipMiMode;

static const FlipMiMode modes[MODE_COUNT] = {
    {
        .label = "Keyboard / YesNo",
        .profile = {
            .name = "FM-KB-YN",
            .appearance = 0x03C1,
            .bonding = true,
            .pairing = GapPairingPinCodeVerifyYesNo,
            .mac_xor = 0x1001,
        },
    },
    {
        .label = "Generic HID / YesNo",
        .profile = {
            .name = "FM-HID-YN",
            .appearance = 0x03C0,
            .bonding = true,
            .pairing = GapPairingPinCodeVerifyYesNo,
            .mac_xor = 0x1002,
        },
    },
    {
        .label = "Remote / YesNo",
        .profile = {
            .name = "FM-RC-YN",
            .appearance = 0x0180,
            .bonding = true,
            .pairing = GapPairingPinCodeVerifyYesNo,
            .mac_xor = 0x1003,
        },
    },
    {
        .label = "Remote / JustWorks",
        .profile = {
            .name = "FM-RC-JW",
            .appearance = 0x0180,
            .bonding = true,
            .pairing = GapPairingNone,
            .mac_xor = 0x1004,
        },
    },
    {
        .label = "Present. / JustWorks",
        .profile = {
            .name = "FM-PR-JW",
            .appearance = 0x03CA,
            .bonding = true,
            .pairing = GapPairingNone,
            .mac_xor = 0x1005,
        },
    },
    {
        .label = "Xiaomi RC / YesNo",
        .profile = {
            .name = "Xiaomi RC",
            .appearance = 0x0180,
            .bonding = true,
            .pairing = GapPairingPinCodeVerifyYesNo,
            .mac_xor = 0x2001,
        },
    },
    {
        .label = "Xiaomi RC / JustWorks",
        .profile = {
            .name = "Xiaomi RC",
            .appearance = 0x0180,
            .bonding = true,
            .pairing = GapPairingNone,
            .mac_xor = 0x2002,
        },
    },
};

typedef struct {
    Bt* bt;
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* input_queue;
    FuriHalBleProfileBase* ble_hid_profile;
    volatile bool connected;
    bool mode_active;
    uint8_t selected_mode;
} FlipMiRemoteApp;

static void flipmiremote_draw_callback(Canvas* canvas, void* context) {
    FlipMiRemoteApp* app = context;

    canvas_clear(canvas);

    if(!app->mode_active) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 10, "FlipMiRemote V0.5");

        canvas_set_font(canvas, FontSecondary);
        const uint8_t visible = 5;
        uint8_t first = 0;
        if(app->selected_mode >= visible) first = app->selected_mode - visible + 1;
        if(first + visible > MODE_COUNT) first = MODE_COUNT - visible;

        for(uint8_t row = 0; row < visible; row++) {
            const uint8_t i = first + row;
            const uint8_t y = 20 + (row * 9);
            canvas_draw_str(canvas, 2, y, (i == app->selected_mode) ? ">" : " ");
            canvas_draw_str(canvas, 10, y, modes[i].label);
        }
    } else {
        const FlipMiMode* mode = &modes[app->selected_mode];

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 11, "BLE test active");

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 25, mode->label);
        canvas_draw_str(canvas, 2, 38, mode->profile.name);
        canvas_draw_str(
            canvas, 2, 51, app->connected ? "Status: CONNECTED" : "Status: advertising");
        canvas_draw_str(canvas, 2, 63, "BACK = modes");
    }
}

static void flipmiremote_input_callback(InputEvent* event, void* context) {
    FlipMiRemoteApp* app = context;
    furi_message_queue_put(app->input_queue, event, 0);
}

static void flipmiremote_bt_status_callback(BtStatus status, void* context) {
    FlipMiRemoteApp* app = context;
    app->connected = (status == BtStatusConnected);
    view_port_update(app->view_port);
}

static bool flipmiremote_start_mode(FlipMiRemoteApp* app) {
    const FlipMiMode* mode = &modes[app->selected_mode];

    bt_disconnect(app->bt);
    furi_delay_ms(200);

    bt_keys_storage_set_storage_path(app->bt, APP_DATA_PATH(HID_BT_KEYS_STORAGE_NAME));

    app->connected = false;
    app->ble_hid_profile =
        bt_profile_start(app->bt, flipmi_ble_profile, (void*)&mode->profile);

    if(!app->ble_hid_profile) {
        FURI_LOG_E(TAG, "Failed to start BLE test mode %u", app->selected_mode);
        bt_keys_storage_set_default_path(app->bt);
        return false;
    }

    bt_set_status_changed_callback(app->bt, flipmiremote_bt_status_callback, app);
    furi_hal_bt_start_advertising();
    app->mode_active = true;

    FURI_LOG_I(TAG, "Started mode %u: %s", app->selected_mode, mode->profile.name);
    view_port_update(app->view_port);
    return true;
}

static void flipmiremote_stop_mode(FlipMiRemoteApp* app) {
    if(!app->mode_active) return;

    bt_set_status_changed_callback(app->bt, NULL, NULL);
    bt_disconnect(app->bt);
    furi_delay_ms(200);

    bt_keys_storage_set_default_path(app->bt);
    furi_check(bt_profile_restore_default(app->bt));

    app->ble_hid_profile = NULL;
    app->connected = false;
    app->mode_active = false;
    view_port_update(app->view_port);
}

int32_t flipmiremote_app(void* p) {
    UNUSED(p);

    FlipMiRemoteApp* app = malloc(sizeof(FlipMiRemoteApp));
    memset(app, 0, sizeof(FlipMiRemoteApp));

    app->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    app->gui = furi_record_open(RECORD_GUI);
    app->bt = furi_record_open(RECORD_BT);

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, flipmiremote_draw_callback, app);
    view_port_input_callback_set(app->view_port, flipmiremote_input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    InputEvent event;
    bool running = true;

    while(running) {
        if(furi_message_queue_get(app->input_queue, &event, 100) != FuriStatusOk) continue;
        if(event.type != InputTypePress) continue;

        if(!app->mode_active) {
            if(event.key == InputKeyUp) {
                app->selected_mode =
                    (app->selected_mode == 0) ? MODE_COUNT - 1 : app->selected_mode - 1;
                view_port_update(app->view_port);
            } else if(event.key == InputKeyDown) {
                app->selected_mode = (app->selected_mode + 1) % MODE_COUNT;
                view_port_update(app->view_port);
            } else if(event.key == InputKeyOk) {
                flipmiremote_start_mode(app);
            } else if(event.key == InputKeyBack) {
                running = false;
            }
        } else if(event.key == InputKeyBack) {
            flipmiremote_stop_mode(app);
        }
    }

    flipmiremote_stop_mode(app);

    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);

    furi_record_close(RECORD_BT);
    furi_record_close(RECORD_GUI);

    furi_message_queue_free(app->input_queue);
    free(app);

    return 0;
}
