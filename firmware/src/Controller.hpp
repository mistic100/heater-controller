#pragma once

#include "constants.h"
#include "model.hpp"
#include "Config.hpp"

class Controller
{
private:
    Config *config;

public:
    Controller(Config *config) : config(config)
    {
        pinMode(PIN_WATER, OUTPUT);
        pinMode(PIN_FP1_NEGATIVE, OUTPUT);
        pinMode(PIN_FP1_POSITIVE, OUTPUT);
        pinMode(PIN_FP2_NEGATIVE, OUTPUT);
        pinMode(PIN_FP2_POSITIVE, OUTPUT);
    }

    void update()
    {
        struct tm time;
        getLocalTime(&time);

        info("Update zone 1");
        Zone zone1 = config->zone1();
        Mode mode1 = getMode(zone1, time);
        apply(PIN_FP1_POSITIVE, PIN_FP1_NEGATIVE, mode1);

        info("Update zone 2");
        Zone zone2 = config->zone2();
        Mode mode2 = getMode(zone2, time);
        apply(PIN_FP2_POSITIVE, PIN_FP2_NEGATIVE, mode2);
    }

private:
    const Mode getMode(const Zone &zone, const struct tm &time) const
    {
        debug("Mode", modeToStr(zone.mode));

        if (zone.mode == AUTO)
        {
            int hour = time.tm_hour;
            bool isWeekend = time.tm_wday == 0 || time.tm_wday == 6;
            auto timetable = isWeekend ? zone.weekend : zone.weekday;

            for (size_t i = timetable.size() - 1; i > 0; i--)
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

        return zone.mode;
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
};