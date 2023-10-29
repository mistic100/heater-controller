#include <Arduino.h>

#include "constants.h"
#include "Config.hpp"
#include "WifiServer.hpp"

Config config;
WifiServer server(&config);

void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("Start");

  if (!SPIFFS.begin(false))
  {
    Serial.println("An Error has occurred while mounting SPIFFS!");
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
  }
}

void loop()
{
  // TODO wifi reconnect
  // TODO OTA
  // TODO Auth
}
