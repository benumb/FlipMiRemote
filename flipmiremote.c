/*
 * FlipMiRemote - BLE HID remote for Xiaomi Mi Box / Android TV
 *
 * V0.2 goal: advertise as a Generic Remote Control rather than a keyboard.
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

typedef struct {
    Bt* bt;
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* input_queue;
    FuriHalBleProfileBase* ble_hid_profile;
    volatile bool connected;
} FlipMiRemoteApp;

static void flipmiremote_draw_callback(Canvas* canvas, void* context) {
    FlipMiRemoteApp* app = context;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "FlipMiRemote V0.2");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 28, app->connected ? "BLE: Connected" : "BLE: Advertising");
    canvas_draw_str(canvas, 2, 40, "Device: FlipMiRemote");
    canvas_draw_str(canvas, 2, 52, "Type: Remote Control");
    canvas_draw_str(canvas, 2, 63, "BACK = Exit");
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

    const FlipMiBleProfileParams profile_params = {
        .name = "FlipMiRemote",
    };

    bt_disconnect(app->bt);
    furi_delay_ms(200);

    bt_keys_storage_set_storage_path(app->bt, APP_DATA_PATH(HID_BT_KEYS_STORAGE_NAME));

    app->ble_hid_profile =
        bt_profile_start(app->bt, flipmi_ble_profile, (void*)&profile_params);

    if(!app->ble_hid_profile) {
        FURI_LOG_E(TAG, "Failed to start BLE HID remote profile");
    } else {
        bt_set_status_changed_callback(app->bt, flipmiremote_bt_status_callback, app);
        furi_hal_bt_start_advertising();
        FURI_LOG_I(TAG, "BLE remote advertising started");
    }

    view_port_update(app->view_port);

    InputEvent event;
    bool running = true;

    while(running) {
        if(furi_message_queue_get(app->input_queue, &event, 100) == FuriStatusOk) {
            if((event.key == InputKeyBack) &&
               ((event.type == InputTypePress) || (event.type == InputTypeShort))) {
                running = false;
            }
        }
    }

    bt_set_status_changed_callback(app->bt, NULL, NULL);
    bt_disconnect(app->bt);
    furi_delay_ms(200);

    bt_keys_storage_set_default_path(app->bt);
    furi_check(bt_profile_restore_default(app->bt));

    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);

    furi_record_close(RECORD_BT);
    furi_record_close(RECORD_GUI);

    furi_message_queue_free(app->input_queue);
    free(app);

    return 0;
}
