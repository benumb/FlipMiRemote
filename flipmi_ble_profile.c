#include "flipmi_ble_profile.h"

#define FLIPMI_REMOTE_APPEARANCE 0x0180

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

    ble_profile_hid->get_gap_config(config, NULL);

    config->appearance_char = FLIPMI_REMOTE_APPEARANCE;

    /* V0.3: emulate a no-input/no-output remote pairing flow. */
    config->bonding_mode = true;
    config->pairing_method = GapPairingNone;

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
