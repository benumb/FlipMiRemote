# FlipMiRemote

**FlipMiRemote** is an experimental Bluetooth remote for Xiaomi Mi Box / Android TV using a Flipper Zero.

## Current status

### V0.2 — Remote Control discovery test

V0.1 compiled successfully, but the Mi Box 4 did not discover the Flipper.

V0.2 changes the Bluetooth identity to better match an Android TV remote:

- standard HID over GATT service;
- Bluetooth GAP appearance **Generic Remote Control (0x0180)** instead of Keyboard;
- explicit BLE name **FlipMiRemote**;
- persistent Bluetooth bonding.

No navigation buttons are implemented yet. The goal of V0.2 is first to confirm discovery and pairing.

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

GitHub Actions also builds the FAP automatically on each push.

## V0.2 test

1. Install the latest `flipmiremote.fap`.
2. Launch **FlipMiRemote**.
3. The Flipper should display **BLE: Advertising**.
4. On the Mi Box 4, open **Settings → Remotes & accessories → Add accessory**.
5. Look specifically for **FlipMiRemote**.
6. Pair it.
7. If successful, the Flipper should display **BLE: Connected**.

## Planned next step

After V0.2 pairing is confirmed:

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
