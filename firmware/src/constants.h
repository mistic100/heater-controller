#pragma once

#define AP_SSID "AP-HEATER"
#define HOSTNAME "heater-ctrl"

#define AUTH_CONFIG_FILE "/auth.txt"
#define WIFI_CONFIG_FILE "/wifi.txt"
#define CONFIG_FILE "/config.json"
#define CONFIG_BUFFER 2048

#define AUTH_DEFAULT_USER "admin"
#define AUTH_DEFAULT_PASS "admin"

#define WIFI_RETRIES 5
#define WIFI_RETRY_INTERVAL 5000

// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"
#define NTP_POOL "pool.ntp.org"

#define KEY_ZONE1 "zone1"
#define KEY_ZONE2 "zone2"
#define KEY_WATER "water"

#define UPDATE_INTERVAL 600000
#define PIN_WATER D2
#define PIN_FP1_POSITIVE D10
#define PIN_FP1_NEGATIVE D3
#define PIN_FP2_POSITIVE D8
#define PIN_FP2_NEGATIVE D9
