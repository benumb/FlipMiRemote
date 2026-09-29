from pathlib import Path

p = Path("targets/f7/ble_glue/gap.c")
s = p.read_text()

fn = "static void gap_advertise_start(GapState new_state) {"
start = s.find(fn)
if start < 0:
    raise SystemExit("gap_advertise_start not found")

marker = "    gap->state = new_state;\n"
pos = s.find(marker, start)
if pos < 0:
    raise SystemExit("gap state marker not found")

injection = r'''    if(!status &&
       (gap->config->adv_service.UUID_Type == UUID_TYPE_16) &&
       (gap->config->adv_service.Service_UUID_16 == HUMAN_INTERFACE_DEVICE_SERVICE_UUID)) {
        uint8_t adv_data[31] = {0};
        uint8_t adv_len = 0;

        adv_data[adv_len++] = 2;
        adv_data[adv_len++] = AD_TYPE_FLAGS;
        adv_data[adv_len++] = 0x02;

        const uint8_t service_len = gap->service.adv_svc_uuid_len;
        if((size_t)(adv_len + 1 + service_len) <= sizeof(adv_data)) {
            adv_data[adv_len++] = service_len;
            memcpy(&adv_data[adv_len], gap->service.adv_svc_uuid, service_len);
            adv_len += service_len;
        }

        const uint8_t name_len = strlen(gap->service.adv_name);
        if((size_t)(adv_len + 1 + name_len) <= sizeof(adv_data)) {
            adv_data[adv_len++] = name_len;
            memcpy(&adv_data[adv_len], gap->service.adv_name, name_len);
            adv_len += name_len;
        }

        status = hci_le_set_advertising_data(adv_len, adv_data);
        if(status) {
            FURI_LOG_E(TAG, "FlipMi HID adv override failed %d", status);
        } else {
            FURI_LOG_I(TAG, "FlipMi HID adv override active: Flags=0x02");
        }
    }
'''

s = s[:pos] + injection + s[pos:]
p.write_text(s)
print("Injected HID Flags 0x02 test override into", p)
