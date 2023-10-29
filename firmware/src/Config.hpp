#pragma once

#include <ArduinoJson.h>
#include <SPIFFS.h>

#include "constants.h"

class Config
{
private:
    String wifi_ssid;
    String wifi_pass;
    StaticJsonDocument<CONFIG_BUFFER> data;

public:
    Config() {}

    const char *ssid() const
    {
        return wifi_ssid.c_str();
    }

    const char *pass() const
    {
        return wifi_pass.c_str();
    }

    bool save()
    {
        File file = SPIFFS.open(CONFIG_FILE, FILE_WRITE);
        if (!file)
        {
            Serial.println("Failed to open config file for writing!");
            return false;
        }

        size_t s = serializeJson(data, file);
        file.close();

        if (s == 0)
        {
            Serial.println("Failed to write config file!");
            return false;
        }
        else
        {
            Serial.println("Saved config file");
            return true;
        }
    }

    bool load()
    {
        File file = SPIFFS.open(CONFIG_FILE);
        if (!file || file.size() == 0)
        {
            Serial.println("config file not found, creating a new one");
            init();
            return save();
        }
        else
        {
            Serial.println("Read config file");

            auto error = deserializeJson(data, file);
            if (error)
            {
                Serial.println("config file invalid!");
            }

            file.close();
            return !error;
        }
    }

    void get(Print &out)
    {
        serializeJson(data, out);
    }

    void set(JsonVariant &json)
    {
        data = json.as<JsonObject>();
        save();
        load(); // reload config from file to release request
    }

    bool loadWifi()
    {
        File file = SPIFFS.open(WIFI_CONFIG_FILE);
        if (!file || file.size() == 0)
        {
            Serial.println("wifi config file not found!");
            return false;
        }

        Serial.println("Read wifi config file");

        if (file.available())
            wifi_ssid = file.readStringUntil('\n');
        if (file.available())
            wifi_pass = file.readStringUntil('\n');
        file.close();

        if (wifi_ssid.isEmpty() || wifi_pass.isEmpty())
        {
            Serial.println("wifi config file empty!");
            return false;
        }

        return true;
    }

    bool saveWifi(String &ssid, String &pass)
    {
        wifi_ssid = ssid;
        wifi_pass = pass;

        File file = SPIFFS.open(WIFI_CONFIG_FILE, FILE_WRITE);
        if (!file)
        {
            Serial.println("Failed to open wifi config file for writing!");
            return false;
        }

        bool ok = file.print(wifi_ssid);
        if (ok)
            ok = file.print('\n');
        if (ok)
            ok = file.print(wifi_pass);
        if (ok)
            ok = file.print('\n');

        file.close();

        if (!ok)
        {
            Serial.println("Failed to write to wifi config file!");
            return false;
        }
        else
        {
            Serial.println("Saved wifi config file");
            return true;
        }
    }

private:
    void init()
    {
        JsonObject zone1 = data.createNestedObject("zone1");
        zone1["mode"] = "auto";
        JsonArray zone1_weekday = zone1.createNestedArray("weekday");
        JsonObject zone1_weekday_0 = zone1_weekday.createNestedObject();
        zone1_weekday_0["hour"] = 0;
        zone1_weekday_0["mode"] = "eco";
        JsonArray zone1_weekend = zone1.createNestedArray("weekend");
        JsonObject zone1_weekend_0 = zone1_weekend.createNestedObject();
        zone1_weekend_0["hour"] = 0;
        zone1_weekend_0["mode"] = "eco";

        JsonObject zone2 = data.createNestedObject("zone2");
        zone2["mode"] = "auto";
        JsonArray zone2_weekday = zone2.createNestedArray("weekday");
        JsonObject zone2_weekday_0 = zone2_weekday.createNestedObject();
        zone2_weekday_0["hour"] = 0;
        zone2_weekday_0["mode"] = "eco";
        JsonArray zone2_weekend = zone2.createNestedArray("weekend");
        JsonObject zone2_weekend_0 = zone2_weekend.createNestedObject();
        zone2_weekend_0["hour"] = 0;
        zone2_weekend_0["mode"] = "eco";
    }
};
