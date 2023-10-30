#include <Arduino.h>

#include "constants.h"
#include "utils.hpp"
#include "Config.hpp"
#include "WifiServer.hpp"
#include "Controller.hpp"

Config config;
WifiServer server(&config);
Controller ctrl(&config);

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

  if (!config.loadWifi())
  {
    server.initAP();
  }
  else
  {
    config.load();
    server.init();
    ctrl.update();
  }
}

unsigned long currentMillis;
unsigned long previousMillis = 0;

void loop()
{
  // TODO wifi reconnect
  // TODO OTA
  // TODO Auth

  currentMillis = millis();
  if (currentMillis - previousMillis >= 60000)
  {
    ctrl.update();
    previousMillis = currentMillis;
  }
}
