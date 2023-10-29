#pragma once

#define AP_SSID "AP-HEATER"
#define HOSTNAME "heater-ctrl"

#define WIFI_CONFIG_FILE "/wifi.txt"
#define CONFIG_FILE "/config.json"
#define CONFIG_BUFFER 2048

#define WIFI_RETRIES 5
#define WIFI_RETRY_INTERVAL 5000

// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"
#define NTP_POOL "pool.ntp.org"
