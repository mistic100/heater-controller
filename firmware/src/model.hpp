#pragma once

#include <Arduino.h>
#include <vector>
#include "utils.hpp"

typedef enum
{
    AUTO,
    ON,
    ECO,
    GEL,
    OFF
} Mode;

typedef struct
{
    u8_t hour;
    Mode mode;
} TimeItem;

typedef struct
{
    Mode mode;
    std::vector<TimeItem> weekday;
    std::vector<TimeItem> weekend;
} Zone;

typedef struct
{
    Mode mode;
    std::vector<TimeItem> week;
} Water;

const String modeToStr(const Mode mode)
{
    switch (mode)
    {
    case AUTO:
        return "auto";
    case ON:
        return "on";
    case ECO:
        return "eco";
    case GEL:
        return "gel";
    case OFF:
        return "off";
    default:
        error("Invalid mode");
        return "on";
    }
}

const Mode strToMode(const char *mode)
{
    if (strcmp(mode, "auto") == 0)
    {
        return AUTO;
    }
    if (strcmp(mode, "on") == 0)
    {
        return ON;
    }
    if (strcmp(mode, "eco") == 0)
    {
        return ECO;
    }
    if (strcmp(mode, "gel") == 0)
    {
        return GEL;
    }
    if (strcmp(mode, "off") == 0)
    {
        return OFF;
    }
    error("Invalid mode");
    return ON;
}
