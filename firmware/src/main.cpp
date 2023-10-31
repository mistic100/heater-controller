#include <Arduino.h>

#include "constants.h"
#include "utils.hpp"
#include "Config.hpp"
#include "WifiServer.hpp"
#include "Controller.hpp"

Config config;
WifiServer server(&config);
Controller ctrl(&config);
bool ap_mode = false;

void setup()
{
  Serial.begin(115200);
  delay(2000);
  info("Start");

  if (!SPIFFS.begin(false))
  {
    error("Failed to init SPIFFS");
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
    ctrl.update();

    server.onUpdate([]() {
      ctrl.update();
    });
  }
}

unsigned long currentMillis;
unsigned long previousMillis = 0;

void loop()
{
  if (!ap_mode)
  {
    currentMillis = millis();
    if (currentMillis - previousMillis >= UPDATE_INTERVAL)
    {
      ctrl.update();
      previousMillis = currentMillis;
    }

    ElegantOTA.loop();
  }
}
