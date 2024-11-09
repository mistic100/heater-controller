#pragma once

#include <ArduinoJson.h>

#include "constants.h"
#include "model.hpp"
#include "Config.hpp"

class Controller
{
private:
    Config *config;

    Mode mode_zone1 = OFF;
    Mode mode_zone2 = OFF;
    Mode mode_water = OFF;

    unsigned long previousMillis = 0;
    bool needsUpdate = false;

public:
    Controller(Config *config) : config(config)
    {
        pinMode(PIN_WATER, OUTPUT);
        pinMode(PIN_FP1_NEGATIVE, OUTPUT);
        pinMode(PIN_FP1_POSITIVE, OUTPUT);
        pinMode(PIN_FP2_NEGATIVE, OUTPUT);
        pinMode(PIN_FP2_POSITIVE, OUTPUT);
    }

    void init()
    {
        needsUpdate = true;
    }

    void loop()
    {
        unsigned long currentMillis = millis();
        if (needsUpdate || (currentMillis - previousMillis >= UPDATE_INTERVAL))
        {
            update();
            previousMillis = currentMillis;
            needsUpdate = false;
        }
    }

    void willUpdate()
    {
        needsUpdate = true;
    }

    void getStatus(Print* output) const
    {
        JsonDocument status;
        status[KEY_ZONE1] = modeToStr(mode_zone1);
        status[KEY_ZONE2] = modeToStr(mode_zone2);
        status[KEY_WATER] = modeToStr(mode_water);
        serializeJson(status, *output);
    }

private:
    void update()
    {
        struct tm time;
        getLocalTime(&time);

        info("Update zone 1");
        Zone zone1 = config->zone1();
        mode_zone1 = getMode(zone1, time);
        apply(PIN_FP1_POSITIVE, PIN_FP1_NEGATIVE, mode_zone1);

        info("Update zone 2");
        Zone zone2 = config->zone2();
        mode_zone2 = getMode(zone2, time);
        apply(PIN_FP2_POSITIVE, PIN_FP2_NEGATIVE, mode_zone2);

        info("Update water");
        Water water = config->water();
        mode_water = getMode(water, time);
        apply(PIN_WATER, mode_water);
    }

    const Mode getMode(const Zone &zone, const struct tm &time) const
    {
        debug("Mode", modeToStr(zone.mode));

        if (zone.mode == AUTO)
        {
            bool isWeekend = time.tm_wday == 0 || time.tm_wday == 6;
            auto timetable = isWeekend ? zone.weekend : zone.weekday;
            return getMode(timetable, time.tm_hour);
        }

        return zone.mode;
    }

    const Mode getMode(const Water &water, const struct tm &time) const
    {
        debug("Mode", modeToStr(water.mode));

        if (water.mode == AUTO)
        {
            return getMode(water.week, time.tm_hour);
        }

        return water.mode;
    }

    const Mode getMode(const std::vector<TimeItem> &timetable, int hour) const
    {
        for (size_t i = timetable.size() - 1; i >= 0; i--)
        {
            if (hour >= timetable[i].hour)
            {
                debug("Scheduled mode", modeToStr(timetable[i].mode));
                return timetable[i].mode;
            }
        }

        error("No suitable timetable found");
        return ON;
    }

    void apply(u8_t pin_positive, u8_t pin_negative, Mode mode)
    {
        switch (mode)
        {
        case ON:
            digitalWrite(pin_positive, LOW);
            digitalWrite(pin_negative, LOW);
            break;
        case ECO:
            digitalWrite(pin_positive, HIGH);
            digitalWrite(pin_negative, HIGH);
            break;
        case GEL:
            digitalWrite(pin_positive, LOW);
            digitalWrite(pin_negative, HIGH);
            break;
        case OFF:
            digitalWrite(pin_positive, HIGH);
            digitalWrite(pin_negative, LOW);
            break;
        default:
            error("Should not be there");
            digitalWrite(pin_positive, LOW);
            digitalWrite(pin_negative, LOW);
            break;
        }
    }

    void apply(u8_t pin, Mode mode)
    {
        switch (mode)
        {
        case ON:
            digitalWrite(pin, HIGH);
            break;
        case OFF:
            digitalWrite(pin, LOW);
            break;
        default:
            error("Should not be there");
            digitalWrite(pin, HIGH);
            break;
        }
    }
};
