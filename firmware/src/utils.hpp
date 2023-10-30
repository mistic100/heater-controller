#pragma once

#include <Arduino.h>

void debug(const char* msg, const String value)
{
    Serial.print("DEBUG: ");
    Serial.print(msg);
    Serial.print(" ");
    Serial.println(value);
}

void info(const char* msg)
{
    Serial.print("INFO: ");
    Serial.println(msg);
}

void error(const char* msg)
{
    Serial.print("ERROR: ");
    Serial.println(msg);
}
