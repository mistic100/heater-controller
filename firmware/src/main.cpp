#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>

#define AP_SSID "AP-HEATER"
#define HOSTNAME "heater-ctrl"
#define WIFI_CONFIG_FILE "/wifi.txt"
#define CONFIG_FILE "/config.json"
#define CONFIG_BUFFER 2048
#define ALLOW_CORS

String ssid;
String pass;
StaticJsonDocument<CONFIG_BUFFER> config;

AsyncWebServer server(80);
bool ap_mode = false;

bool readWifiConfig() {
  File file = SPIFFS.open(WIFI_CONFIG_FILE);
  if (!file || file.size() == 0) {
    Serial.println("wifi.txt not found");
    return false;
  }

  Serial.println("Read wifi.txt");

  if (file.available()) {
    ssid = file.readStringUntil('\n');
  }
  if (file.available()) {
    pass = file.readStringUntil('\n');
  }

  file.close();

  if (ssid.isEmpty() || pass.isEmpty()) {
    Serial.println("wifi.txt empty");
    return false;
  }

  return true;
}

bool writeWifiConfig() {
  File file = SPIFFS.open(WIFI_CONFIG_FILE, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open wifi.txt for writing");
    return false;
  }

  bool ok = file.print(ssid);
  if (ok) ok = file.print('\n');
  if (ok) ok = file.print(pass);
  if (ok) ok = file.print('\n');

  file.close();

  if (!ok) {
    Serial.println("Failed to write to wifi.txt");
    return false;
  }

  Serial.println("Saved wifi.txt");

  return ok;
}

void saveConfig() {
  File file = SPIFFS.open(CONFIG_FILE, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open config.json for writing");
    return;
  }

  serializeJson(config, file);
  file.close();

  Serial.println("Saved config.json");
}

void readConfig() {
  File file = SPIFFS.open(CONFIG_FILE);
  if (!file || file.size() == 0) {
    Serial.println("config.json file not found, creating a new one");

    JsonObject zone1 = config.createNestedObject("zone1");
    zone1["mode"] = "auto";
    JsonArray zone1_weekday = zone1.createNestedArray("weekday");
    JsonObject zone1_weekday_0 = zone1_weekday.createNestedObject();
    zone1_weekday_0["hour"] = 0;
    zone1_weekday_0["mode"] = "eco";
    JsonArray zone1_weekend = zone1.createNestedArray("weekend");
    JsonObject zone1_weekend_0 = zone1_weekend.createNestedObject();
    zone1_weekend_0["hour"] = 0;
    zone1_weekend_0["mode"] = "eco";

    JsonObject zone2 = config.createNestedObject("zone2");
    zone2["mode"] = "auto";
    JsonArray zone2_weekday = zone2.createNestedArray("weekday");
    JsonObject zone2_weekday_0 = zone2_weekday.createNestedObject();
    zone2_weekday_0["hour"] = 0;
    zone2_weekday_0["mode"] = "eco";
    JsonArray zone2_weekend = zone2.createNestedArray("weekend");
    JsonObject zone2_weekend_0 = zone2_weekend.createNestedObject();
    zone2_weekend_0["hour"] = 0;
    zone2_weekend_0["mode"] = "eco";

    saveConfig();
  } else {
    Serial.println("Read config.json");

    if (deserializeJson(config, file)) {
      Serial.println("config.json invalid");
    }

    file.close();
  }
}

void initAP() {
  Serial.println("Starting AP");

  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(AP_SSID, NULL)) {
    Serial.println("Failed to start AP");
  } else {
    Serial.print("AP ssid: ");
    Serial.println(AP_SSID);
    Serial.print("AP ip: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/wifimanager.html", "text/html");
    });

    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for (int i=0; i<params; i++) {
        AsyncWebParameter* p = request->getParam(i);
        if (p->isPost()) {
          if (p->name() == "ssid") ssid = p->value().c_str();
          if (p->name() == "pass") pass = p->value().c_str();
        }
      }
      if (writeWifiConfig()) {
        request->send(200, "text/plain", "Done. ESP will restart.");
        delay(3000);
        ESP.restart();
      } else {
        request->send(500, "text/plain", "Failed to store settings.");
      }
    });

    server.begin();

    ap_mode = true;
  }
}

void initWifi() {
  Serial.print("Connecting to WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  unsigned long currentMillis = millis();
  unsigned long previousMillis = currentMillis;
  unsigned int triesCount = 0;

  while (WiFi.status() != WL_CONNECTED && triesCount < 10) {
    delay(100);
    currentMillis = millis();
    if (currentMillis - previousMillis >= 5000) {
      Serial.print(".");
      WiFi.reconnect();
      previousMillis = currentMillis;
      triesCount++;
    }
  }

  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Failed to connect");
      initAP();
      return;
  }

  Serial.print("ip: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin(HOSTNAME)) {
    Serial.println("Failed to start mDNS");
  } else {
    Serial.print("hostname: ");
    Serial.println(HOSTNAME);
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html");
  });

  #ifdef ALLOW_CORS
  server.on("/config.json", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "*");
    response->addHeader("Access-Control-Allow-Headers", "*");
    request->send(response);
  });
  #endif

  server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    #ifdef ALLOW_CORS
    response->addHeader("Access-Control-Allow-Origin", "*");
    #endif
    serializeJson(config, *response);
    request->send(response);
  });

  server.addHandler(new AsyncCallbackJsonWebHandler("/config.json", [](AsyncWebServerRequest *request, JsonVariant &json) {
    config = json.as<JsonObject>();
    saveConfig();
    readConfig(); // re-read config from file to release request memory

    AsyncWebServerResponse *response = request->beginResponse(200);
    #ifdef ALLOW_CORS
    response->addHeader("Access-Control-Allow-Origin", "*");
    #endif
    request->send(200);
  }, CONFIG_BUFFER));

  server.begin();
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Start");

  if (!SPIFFS.begin(false)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  if (!readWifiConfig()) {
    initAP();
  } else {
    readConfig();
    initWifi();
  }
}

void loop () {
  // TODO wifi reconnect
  // TODO OTA
  // TODO Auth
}
