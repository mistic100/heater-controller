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
    std::function<void()> callback;

public:
    WifiServer(Config *config) : config(config) {}

    void onUpdate(std::function<void()> cb) {
        callback = cb;
    }

    void initAP()
    {
        info("Starting AP");

        WiFi.mode(WIFI_AP);
        if (!WiFi.softAP(AP_SSID, NULL))
        {
            error("Failed to start AP");
        }
        else
        {
            debug("ssid", AP_SSID);
            debug("IP", WiFi.softAPIP().toString());

            serveAP();
        }
    }

    void init()
    {
        info("Connecting to WiFi");

        WiFi.mode(WIFI_STA);
        WiFi.begin(config->wifi_ssid(), config->wifi_pass());

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

        if (triesCount > 0)
        {
            Serial.println();
        }

        if (WiFi.status() != WL_CONNECTED)
        {
            error("Failed to connect");
            initAP();
            return;
        }

        debug("IP", WiFi.localIP().toString());

        if (!MDNS.begin(HOSTNAME))
        {
            error("Failed to start mDNS");
        }
        else
        {
            debug("Hostname", HOSTNAME);
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
            error("Failed to obtain time");
        }
        else
        {
            char buf[64];
            size_t written = strftime(buf, 64, "%Y-%m-%d %H:%M:%S", &timeinfo);
            debug("Current time", String(buf));
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
        server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
            if (!request->authenticate(config->auth_user(), config->auth_pass()))
            {
                return request->requestAuthentication();
            }
            
            request->send(SPIFFS, "/index.html", "text/html");
        });

        server.on("/config.json", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
            AsyncWebServerResponse *response = request->beginResponse(200);
            response->addHeader("Access-Control-Allow-Methods", "*");
            response->addHeader("Access-Control-Allow-Headers", "*");
            request->send(response);
        });

        server.on("/config.json", HTTP_GET, [this](AsyncWebServerRequest *request) {
            if (!request->authenticate(config->auth_user(), config->auth_pass()))
            {
                return request->requestAuthentication();
            }
            
            request->send(SPIFFS, CONFIG_FILE, "application/json");
        });

        server.on("/settings/auth", HTTP_POST, [this](AsyncWebServerRequest *request) {
            if (!request->authenticate(config->auth_user(), config->auth_pass()))
            {
                return request->requestAuthentication();
            }
            
            String user;
            String pass;

            int params = request->params();
            for (int i=0; i<params; i++)
            {
                AsyncWebParameter* p = request->getParam(i);
                if (p->isPost())
                {
                    if (p->name() == "user") user = p->value().c_str();
                    if (p->name() == "pass") pass = p->value().c_str();
                }
            }

            if (config->saveAuth(user, pass))
            {
                request->redirect("/");
            }
            else
            {
                request->send(500, "text/plain", "Failed to store settings!");
            }
        });

        server.addHandler(new AsyncCallbackJsonWebHandler("/config.json", [this](AsyncWebServerRequest *request, JsonVariant &json) {
            if (!request->authenticate(config->auth_user(), config->auth_pass()))
            {
                return request->requestAuthentication();
            }
            
            config->set(json);
            request->send(200);

            if (callback) callback();
        }, CONFIG_BUFFER));

        server.begin();
    }
};
