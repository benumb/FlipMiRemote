#pragma once

#include <furi.h>
#include <furi_hal_bt.h>
#include <extra_profiles/hid_profile.h>

typedef struct {
    const char* name;
    uint16_t appearance;
    bool bonding;
    GapPairing pairing;
    uint16_t mac_xor;
} FlipMiBleProfileParams;

extern const FuriHalBleProfileTemplate* flipmi_ble_profile;
