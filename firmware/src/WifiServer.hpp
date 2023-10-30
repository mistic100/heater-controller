#pragma once

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <time.h>

#include "Config.hpp"
#include "constants.h"

class WifiServer
{
private:
    Config *config;
    AsyncWebServer server = AsyncWebServer(80);

public:
    WifiServer(Config *config) : config(config) {}

    void initAP()
    {
        Serial.println("Starting AP");

        WiFi.mode(WIFI_AP);
        if (!WiFi.softAP(AP_SSID, NULL))
        {
            Serial.println("Failed to start AP");
        }
        else
        {
            Serial.print("ssid: ");
            Serial.println(AP_SSID);
            Serial.print("ip: ");
            Serial.println(WiFi.softAPIP());

            serveAP();
        }
    }

    void init()
    {
        Serial.print("Connecting to WiFi");

        WiFi.mode(WIFI_STA);
        WiFi.begin(config->ssid(), config->pass());

        unsigned long currentMillis = millis();
        unsigned long previousMillis = currentMillis;
        unsigned int triesCount = 0;

        while (WiFi.status() != WL_CONNECTED && triesCount < WIFI_RETRIES)
        {
            delay(100);
            currentMillis = millis();
            if (currentMillis - previousMillis >= WIFI_RETRY_INTERVAL)
            {
                Serial.print(".");
                WiFi.reconnect();
                previousMillis = currentMillis;
                triesCount++;
            }
        }

        Serial.println();

        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Failed to connect!");
            initAP();
            return;
        }

        Serial.print("ip: ");
        Serial.println(WiFi.localIP());

        if (!MDNS.begin(HOSTNAME))
        {
            Serial.println("Failed to start mDNS!");
        }
        else
        {
            Serial.print("hostname: ");
            Serial.println(HOSTNAME);
        }

        serve();
        syncTime();
    }

    void syncTime()
    {
        configTime(0, 0, NTP_POOL);
        setenv("TZ", TIMEZONE, 1);
        tzset();

        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("Failed to obtain time!");
        } else {
            Serial.print("current time: ");
            Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
        }
    }

private:
    void serveAP()
    {
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { 
            request->send(SPIFFS, "/wifimanager.html", "text/html");
        });

        server.on("/", HTTP_POST, [this](AsyncWebServerRequest *request) {
            String ssid;
            String pass;

            int params = request->params();
            for (int i=0; i<params; i++)
            {
                AsyncWebParameter* p = request->getParam(i);
                if (p->isPost())
                {
                    if (p->name() == "ssid") ssid = p->value().c_str();
                    if (p->name() == "pass") pass = p->value().c_str();
                }
            }

            if (config->saveWifi(ssid, pass))
            {
                request->send(200, "text/plain", "Done. ESP will restart.");
                delay(3000);
                ESP.restart();
            }
            else
            {
                request->send(500, "text/plain", "Failed to store settings!");
            }
        });

        server.begin();
    }

    void serve()
    {
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/index.html", "text/html");
        });

        server.on("/config.json", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
            AsyncWebServerResponse *response = request->beginResponse(200);
            response->addHeader("Access-Control-Allow-Methods", "*");
            response->addHeader("Access-Control-Allow-Headers", "*");
            request->send(response);
        });

        server.on("/config.json", HTTP_GET, [this](AsyncWebServerRequest *request) {
            request->send(SPIFFS, CONFIG_FILE, "application/json");
        });

        server.addHandler(new AsyncCallbackJsonWebHandler("/config.json", [this](AsyncWebServerRequest *request, JsonVariant &json) {
            config->set(json);

            request->send(200);
        }, CONFIG_BUFFER));

        server.begin();
    }
};
