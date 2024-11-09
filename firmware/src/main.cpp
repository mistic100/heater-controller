#include <Arduino.h>
#include <ElegantOTA.h>
#include <LittleFS.h>
#include "constants.h"
#include "utils.hpp"
#include "Config.hpp"
#include "WifiServer.hpp"
#include "Controller.hpp"

Config config;
Controller ctrl(&config);
WifiServer server(&config, &ctrl);
bool ap_mode = false;

void setup()
{
    Serial.begin(115200);
    delay(2000);
    info("Start");

    if (!LittleFS.begin(false))
    {
        error("Failed to init LittleFS");
        return;
    }

    if (!config.loadAuth())
    {
        info("Create default auth file");
        config.saveAuth(AUTH_DEFAULT_USER, AUTH_DEFAULT_PASS);
    }

    if (!config.loadWifi())
    {
        ap_mode = true;
        server.initAP();
    }
    else
    {
        config.load();
        server.init();
        ctrl.init();
    }
}

void loop()
{
    if (!ap_mode)
    {
        ctrl.loop();
        ElegantOTA.loop();
    }
}
