#pragma once

#include <ArduinoJson.h>
#include <SPIFFS.h>

#include "constants.h"
#include "model.hpp"

class Config
{
private:
    String _ssid;
    String _pass;
    Zone _zone1;
    Zone _zone2;
    Water _water;

public:
    Config() {}

    const char *ssid() const
    {
        return _ssid.c_str();
    }

    const char *pass() const
    {
        return _pass.c_str();
    }

    const Zone& zone1() const
    {
        return _zone1;
    }

    const Zone& zone2() const
    {
        return _zone2;
    }

    const Water& water() const
    {
        return _water;
    }

    bool save()
    {
        File file = SPIFFS.open(CONFIG_FILE, FILE_WRITE);
        if (!file)
        {
            error("Failed to open config file for writing");
            return false;
        }

        DynamicJsonDocument data(CONFIG_BUFFER);
        JsonObject zone1_data = data.createNestedObject("zone1");
        serialize(_zone1, zone1_data);
        JsonObject zone2_data = data.createNestedObject("zone2");
        serialize(_zone2, zone2_data);
        JsonObject water_data = data.createNestedObject("water");
        serialize(_water, water_data);

        size_t s = serializeJson(data, file);
        file.close();

        if (s == 0)
        {
            error("Failed to write config file");
            return false;
        }
        else
        {
            info("Saved config file");
            return true;
        }
    }

    bool load()
    {
        File file = SPIFFS.open(CONFIG_FILE);
        if (!file || file.size() == 0)
        {
            info("config file not found, creating a new one");
            init();
            return save();
        }
        else
        {
            info("Read config file");

            DynamicJsonDocument data(CONFIG_BUFFER);
            auto nok = deserializeJson(data, file);
            if (nok)
            {
                error("Config file invalid");
            }
            else
            {
                deserialize(_zone1, data["zone1"]);
                deserialize(_zone2, data["zone2"]);
                deserialize(_water, data["water"]);
            }

            file.close();
            return !nok;
        }
    }

    void set(const JsonVariant &json)
    {
        JsonObjectConst data = json.as<JsonObjectConst>();
        deserialize(_zone1, data["zone1"]);
        deserialize(_zone2, data["zone2"]);
        deserialize(_water, data["water"]);
        save();
    }

    bool loadWifi()
    {
        File file = SPIFFS.open(WIFI_CONFIG_FILE);
        if (!file || file.size() == 0)
        {
            error("wifi config file not found");
            return false;
        }

        info("Read wifi config file");

        if (file.available()) _ssid = file.readStringUntil('\n');
        if (file.available()) _pass = file.readStringUntil('\n');
        file.close();

        if (_ssid.isEmpty() || _pass.isEmpty())
        {
            error("Wifi config file empty");
            return false;
        }

        return true;
    }

    bool saveWifi(String &ssid, String &pass)
    {
        if (ssid.isEmpty() || pass.isEmpty())
        {
            error("Invalid ssid/password");
            return false;
        }

        _ssid = ssid;
        _pass = pass;

        File file = SPIFFS.open(WIFI_CONFIG_FILE, FILE_WRITE);
        if (!file)
        {
            error("Failed to open wifi config file for writing");
            return false;
        }

        bool ok = file.print(_ssid);
        if (ok) ok = file.print('\n');
        if (ok) ok = file.print(_pass);
        if (ok) ok = file.print('\n');

        file.close();

        if (!ok)
        {
            error("Failed to write to wifi config file");
            return false;
        }
        else
        {
            info("Saved wifi config file");
            return true;
        }
    }

private:
    void init()
    {
        _zone1.mode = AUTO;
        _zone1.weekday.push_back({0, OFF});
        _zone1.weekend.push_back({0, OFF});
        _zone2.mode = AUTO;
        _zone2.weekday.push_back({0, OFF});
        _zone2.weekend.push_back({0, OFF});
        _water.mode = AUTO;
        _water.week.push_back({0, OFF});
    }

    void deserialize(Zone &zone, const JsonObjectConst &data)
    {
        zone.mode = strToMode(data["mode"]);
        deserialize(zone.weekday, data["weekday"].as<JsonArrayConst>());
        deserialize(zone.weekend, data["weekend"].as<JsonArrayConst>());
    }

    void deserialize(Water &water, const JsonObjectConst &data)
    {
        water.mode = strToMode(data["mode"]);
        deserialize(water.week, data["week"].as<JsonArrayConst>());;
    }

    void deserialize(std::vector<TimeItem> &items, const JsonArrayConst &data)
    {
        items.clear();
        for (size_t i = 0; i < data.size(); i++)
        {
            items.push_back({(u8_t)data[i]["hour"], strToMode(data[i]["mode"])});
        }
    }

    void serialize(const Zone &zone, JsonObject &data)
    {
        data["mode"] = modeToStr(zone.mode);
        JsonArray weekday_data = data.createNestedArray("weekday");
        serialize(zone.weekday, weekday_data);
        JsonArray weekend_data = data.createNestedArray("weekend");
        serialize(zone.weekend, weekend_data);
    }

    void serialize(const Water &water, JsonObject &data)
    {
        data["mode"] = modeToStr(water.mode);
        JsonArray week_data = data.createNestedArray("week");
        serialize(water.week, week_data);
    }

    void serialize(const std::vector<TimeItem> &items, JsonArray &data)
    {
        for (uint i = 0; i < items.size(); i++)
        {
            data[i]["hour"] = items[i].hour;
            data[i]["mode"] = modeToStr(items[i].mode);
        }
    }
};
