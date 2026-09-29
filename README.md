# FlipMiRemote

**FlipMiRemote** is an experimental Bluetooth remote for Xiaomi Mi Box / Android TV using a Flipper Zero.

## Current status

### V0.1 — BLE discovery test

The first milestone is deliberately minimal:

- start the Flipper Zero as a BLE HID device;
- advertise it to nearby Bluetooth hosts;
- verify that a Xiaomi Mi Box 4 can see it;
- verify that pairing succeeds;
- preserve the Bluetooth bond between launches.

No navigation buttons are implemented yet. They will only be added after BLE discovery/pairing is confirmed on real Mi Box hardware.

## Bluetooth name

Momentum's current public HID profile limits the custom device-name prefix to fewer than 8 characters.

For V0.1:

- app name on Flipper: **FlipMiRemote**
- BLE advertising name: **FlipMi <Flipper device name>**

On the Mi Box, look for a Bluetooth device beginning with **FlipMi**.

## Requirement

- Flipper Zero
- Momentum firmware
- Xiaomi Mi Box / Android TV device to test

## Build

Clone Momentum firmware and place/symlink this repository in `applications_user/flipmiremote`, then build:

```bash
git clone --recursive https://github.com/Next-Flip/Momentum-Firmware.git
cd Momentum-Firmware
ln -s /path/to/FlipMiRemote applications_user/flipmiremote
./fbt fap_flipmiremote
```

The resulting `.fap` can be copied to:

```text
SD:/apps/Bluetooth/
```

## V0.1 test

1. Launch **FlipMiRemote** on the Flipper.
2. The screen should show **BLE: Advertising**.
3. On the Mi Box 4, open Bluetooth / Add accessory.
4. Look for a device beginning with **FlipMi**.
5. Pair it.
6. If successful, the Flipper should display **BLE: Connected**.

## Planned next step

After pairing is confirmed:

- D-pad: Up / Down / Left / Right
- OK / Select
- Back
- Home
- Play / Pause
- Volume Up / Down

## Credits

The project was inspired by the BLE HID approach used by the Apple TV Remote project from 1507-systems and by the HID implementation available in Momentum firmware.

- Upstream inspiration: `1507-systems/flipper-apple-tv-remote`
- Firmware target: `Next-Flip/Momentum-Firmware`

## License

GPL-2.0.
