#include "flipmi_ble_profile.h"

static FuriHalBleProfileBase* flipmi_profile_start(FuriHalBleProfileParams profile_params) {
    UNUSED(profile_params);
    return ble_profile_hid->start(NULL);
}

static void flipmi_profile_stop(FuriHalBleProfileBase* profile) {
    ble_profile_hid->stop(profile);
}

static void flipmi_profile_get_config(
    GapConfig* config,
    FuriHalBleProfileParams profile_params) {

    furi_check(config);
    furi_check(profile_params);

    const FlipMiBleProfileParams* params = profile_params;

    /* Start from Momentum's standard HID-over-GATT profile. */
    ble_profile_hid->get_gap_config(config, NULL);

    config->appearance_char = params->appearance;
    config->bonding_mode = params->bonding;
    config->pairing_method = params->pairing;

    /* Give every test mode a distinct BLE identity. */
    config->mac_address[0] ^= params->mac_xor & 0xFF;
    config->mac_address[1] ^= (params->mac_xor >> 8) & 0xFF;

    if(params->name && params->name[0] != '\0') {
        strlcpy(config->adv_name + 1, params->name, sizeof(config->adv_name) - 1);
    }
}

static const FuriHalBleProfileTemplate flipmi_profile_callbacks = {
    .start = flipmi_profile_start,
    .stop = flipmi_profile_stop,
    .get_gap_config = flipmi_profile_get_config,
};

const FuriHalBleProfileTemplate* flipmi_ble_profile = &flipmi_profile_callbacks;
