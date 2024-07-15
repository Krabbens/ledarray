#pragma once

#include <WiFi.h>
#include <Preferences.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include "ESPAsyncWebServer.h"
#include "Debug.h"

class Wireless {
public:
    Wireless();
    ~Wireless();

    void connectToNetwork();
    void startAP();

    bool isConnected() {
        return WiFi.status() == WL_CONNECTED;
    }
    
private:
    Preferences preferences;
    DNSServer dnsServer;
    AsyncWebServer* server;
    String msg_ssid, msg_password;

    bool serverRunning = false;
};

Wireless::Wireless() {
    Debug::info("Wireless object created.");
    preferences.begin("wireless", false);
    if (preferences.isKey("ssid")) {
        Debug::info("Last known network found.");
        connectToNetwork();
    }
    else {
        Debug::info("No last known network found.");
        startAP();
    }
}


Wireless::~Wireless() {
    Debug::info("Wireless object destroyed.");
    if (server) {
        delete server;
    }
}

void Wireless::connectToNetwork() {
    Debug::info("Connecting to network...");
    String ssid = preferences.getString("ssid");
    String password = preferences.getString("password");
    Debug::info("SSID: " + ssid);
    Debug::info("Password: " + password);
    WiFi.begin(preferences.getString("ssid").c_str(), preferences.getString("password").c_str());
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Debug::info("Connecting...");
        retries++;
        if (retries > 6) {
            Debug::error("Failed to connect to network.");
            if (serverRunning) {
                return;
            }
            startAP();
            return;
        }
    }
    Debug::info("Connected to network.");
    preferences.end();
}

void Wireless::startAP() {
    Debug::info("Starting AP...");
    serverRunning = true;
    WiFi.softAP("ledarray");
    dnsServer.start(53, "*", WiFi.softAPIP());
    server = new AsyncWebServer(80);
    
    /*
        Create listener for new network credentials.
    */
    server->on("/get", HTTP_GET, [&] (AsyncWebServerRequest *request) {
        
        if (request->hasParam("ssid") && request->hasParam("password")) {
            Debug::info("SSID and password provided.");
            msg_ssid = request->getParam("ssid")->value();
            msg_password = request->getParam("password")->value();
            Debug::info("SSID: " + msg_ssid);
            Debug::info("Password: " + msg_password);
            preferences.putString("ssid", msg_ssid);
            preferences.putString("password", msg_password);
            
            connectToNetwork();
        } else {
            Debug::error("No ssid or password provided.");
        }
        request->send(200, "text/plain", "OK");
    });
    server->on("/status", HTTP_GET, [&] (AsyncWebServerRequest *request) {
        Debug::info("ESP32 IP on the WiFi network: ");
        Debug::info(WiFi.localIP().toString());
        request->send(200, "text/plain", "OK");
    });
    server->begin();
}