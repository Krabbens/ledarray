#pragma once

#include <WiFi.h>
#include <Preferences.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include "ESPAsyncWebServer.h"
#include "Debug.h"

const char* PARAM_MESSAGE = "message";

class Wireless {
public:
    Wireless();
    ~Wireless();

private:
    Preferences preferences;
    DNSServer dnsServer;
    AsyncWebServer* server;
};

Wireless::Wireless() {
    Debug::info("Wireless object created.");
    preferences.begin("wireless", false);
    if (preferences.isKey("ssid")) {
        //try to connect to the last known network
        Debug::info("Connecting to last known network...");
        int retries = 0;
        auto status = WiFi.begin(preferences.getString("ssid").c_str(), preferences.getString("password").c_str());
        while (status != WL_CONNECTED) {
            delay(500);
            Debug::info("Connecting...");
            status = WiFi.status();
            retries++;
            if (retries > 6) {
                Debug::error("Failed to connect to last known network.");
                goto ap;
            }
        }
    }
    else {
        ap:
        Debug::info("Starting AP...");
        // start AP
        WiFi.softAP("ledarray");
        dnsServer.start(53, "*", WiFi.softAPIP());
        server = new AsyncWebServer(80);
        server->on("/get", HTTP_GET, [&] (AsyncWebServerRequest *request) {
            String msg_ssid, msg_password;
            if (request->hasParam("ssid") && request->hasParam("password")) {
                Debug::info("SSID and password provided.");
                msg_ssid = request->getParam("ssid")->value();
                msg_password = request->getParam("password")->value();
                preferences.putString("ssid", msg_ssid);
                preferences.putString("password", msg_password);
                ESP.restart();
            } else {
                Debug::error("No ssid or password provided.");
            }
            request->send(200, "text/plain", "OK");
        });
        server->begin();
    }
}


Wireless::~Wireless() {
    preferences.end();
}