#pragma once

#include <WiFi.h>
#include <Preferences.h>
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
    void stopAP();
    bool runningAP();             
    bool isConnected();        

    void clearCredentials();   
    
private:
    Preferences preferences;
    DNSServer dnsServer;
    AsyncWebServer* server;
    String msg_ssid, msg_password;
    bool serverRunning = false;

    bool attemptConnection(const String& ssid, const String& password);
    
    void setupWebServer();
};

Wireless::Wireless() {
    Debug::info("Wireless object created.");
    preferences.begin("wireless", false);
}

Wireless::~Wireless() {
    Debug::info("Wireless object destroyed.");
    if (server) {
        delete server;
    }
    preferences.end();
}

bool Wireless::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void Wireless::connectToNetwork() {
    if (!preferences.isKey("ssid")) {
        Debug::error("No saved WiFi credentials.");
        return;
    }

    String ssid = preferences.getString("ssid");
    String password = preferences.getString("password");

    Debug::info("Attempting to connect to network with SSID: " + ssid);
    bool connected = attemptConnection(ssid, password);

    if (connected) {
        Debug::info("Successfully connected to the network.");
    } else {
        Debug::error("Failed to connect to the network.");
    }
}

bool Wireless::attemptConnection(const String& ssid, const String& password) {
    WiFi.begin(ssid.c_str(), password.c_str());
    int retries = 0;

    while (WiFi.status() != WL_CONNECTED && retries < 10) {
        delay(1000); 
        Debug::info("Connecting... attempt: " + String(retries));
        retries++;
    }

    Debug::info("Id: " + String(WiFi.localIP()));

    return WiFi.status() == WL_CONNECTED;
}

void Wireless::startAP() {
    Debug::info("Starting Access Point (AP) mode...");
    serverRunning = true;
    WiFi.softAP("ledarray");

    dnsServer.start(53, "*", WiFi.softAPIP());

    setupWebServer(); 
}

void Wireless::stopAP() {
    if (serverRunning) {
        WiFi.softAPdisconnect(true); 
        dnsServer.stop();            
        Debug::info("Access Point stopped.");
        serverRunning = false;
    } else {
        Debug::info("Access Point is not running.");
    }
}

bool Wireless::runningAP(){
    return serverRunning;
}

void Wireless::setupWebServer() {
    server = new AsyncWebServer(80);

    server->on("/register", HTTP_GET, [&](AsyncWebServerRequest *request) {
        if (request->hasParam("ssid") && request->hasParam("password")) {
            Debug::info("Received new SSID and password.");
            msg_ssid = request->getParam("ssid")->value();
            msg_password = request->getParam("password")->value();

            preferences.putString("ssid", msg_ssid);
            preferences.putString("password", msg_password);

            bool connected = attemptConnection(msg_ssid, msg_password);

            if (connected) {
                request->send(200, "text/plain", "Connected.");
            } else {
                request->send(400, "text/plain", "Failed to connect.");
            }
        } else {
            Debug::error("Missing SSID or password.");
            request->send(400, "text/plain", "No SSID or password provided.");
        }
    });

    server->on("/status", HTTP_GET, [&](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "ESP32 IP: " + WiFi.localIP().toString());
    });

    server->begin();
}

void Wireless::clearCredentials() {
    preferences.clear();
    Debug::info("Cleared stored WiFi credentials.");
}
