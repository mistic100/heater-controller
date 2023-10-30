#pragma once

#include <ArduinoJson.h>
#include <SPIFFS.h>

#include "constants.h"
#include "model.hpp"

class Config
{
private:
    String wifi_ssid;
    String wifi_pass;
    Zone zone1;
    Zone zone2;

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

        DynamicJsonDocument data(CONFIG_BUFFER);
        JsonObject zone1_data = data.createNestedObject("zone1");
        serializeZone(zone1, zone1_data);
        JsonObject zone2_data = data.createNestedObject("zone2");
        serializeZone(zone2, zone2_data);

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

            DynamicJsonDocument data(CONFIG_BUFFER);
            auto error = deserializeJson(data, file);
            if (error)
            {
                Serial.println("config file invalid!");
            }
            else
            {
                deserializeZone(zone1, data["zone1"]);
                deserializeZone(zone2, data["zone2"]);
            }

            file.close();
            return !error;
        }
    }

    void set(JsonVariant &json)
    {
        JsonObjectConst data = json.as<JsonObjectConst>();
        deserializeZone(zone1, data["zone1"]);
        deserializeZone(zone2, data["zone2"]);
        save();
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
        zone1.mode = AUTO;
        zone1.weekday.push_back({0, ECO});
        zone1.weekend.push_back({0, ECO});
        zone2.mode = AUTO;
        zone2.weekday.push_back({0, ECO});
        zone2.weekend.push_back({0, ECO});
    }

    void deserializeZone(Zone &zone, const JsonObjectConst &data)
    {
        zone.mode = strToMode(data["mode"]);
        deserializeTimeItems(zone.weekday, data["weekday"].as<JsonArrayConst>());
        deserializeTimeItems(zone.weekend, data["weekend"].as<JsonArrayConst>());
    }

    void deserializeTimeItems(std::vector<TimeItem> &items, const JsonArrayConst &data)
    {
        items.clear();
        for (size_t i = 0; i < data.size(); i++)
        {
            items.push_back({(u8_t)data[i]["hour"], strToMode(data[i]["mode"])});
        }
    }

    void serializeZone(const Zone &zone, JsonObject &data)
    {
        data["mode"] = modeToStr(zone.mode);
        JsonArray weekday_data = data.createNestedArray("weekday");
        serializeTimeItems(zone.weekday, weekday_data);
        JsonArray weekend_data = data.createNestedArray("weekend");
        serializeTimeItems(zone.weekend, weekend_data);
    }

    void serializeTimeItems(const std::vector<TimeItem> &items, JsonArray &data)
    {
        for (uint i = 0; i < items.size(); i++)
        {
            data[i]["hour"] = items[i].hour;
            data[i]["mode"] = modeToStr(items[i].mode);
        }
    }
};
