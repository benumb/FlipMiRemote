#pragma once

#include <furi.h>
#include <furi_hal_bt.h>
#include <extra_profiles/hid_profile.h>

typedef struct {
    const char* name;
} FlipMiBleProfileParams;

extern const FuriHalBleProfileTemplate* flipmi_ble_profile;
