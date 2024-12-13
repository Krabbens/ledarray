#pragma once

#include <WiFi.h>
#include <Preferences.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include "ESPAsyncWebServer.h"
#include "Debug.h"
#include <WiFiUdp.h>
#include "esp_task_wdt.h"

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

    WiFiUDP udp; // Obiekt do wysyłania danych UDP

    bool attemptConnection(const String& ssid, const String& password);

    void setupWebServer();
    void sendBroadcast(const String& ssid, const String& password);
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

bool Wireless:: attemptConnection(const String& ssid, const String& password) {
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
        server->end();
        free(server);
        Debug::info("Access Point stopped.");
        serverRunning = false;
    } else {
        Debug::info("Access Point is not running.");
    }
}

bool Wireless::runningAP() {
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

            WiFi.begin(msg_ssid.c_str(), msg_password.c_str());
            int retries = 0;

            while (WiFi.status() != WL_CONNECTED && retries < 10) {
                esp_task_wdt_reset();
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                Debug::info("ASYNC Connecting... attempt: " + String(retries));
                retries++;
            }

            bool connected = WiFi.status();

            if (connected) {
                request->send(200, "text/plain", "Connected.");
                sendBroadcast(msg_ssid, msg_password);
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

void Wireless::sendBroadcast(const String& ssid, const String& password) {

    const char* broadcastAddress = "255.255.255.255";
    uint16_t port = 12345;

    udp.begin(WiFi.localIP());

    Debug::info("Sending broadcast with SSID: " + ssid + " and Password: " + password);
    udp.beginPacket(broadcastAddress, port);
    
    udp.write((uint8_t*)ssid.c_str(), ssid.length());
    udp.write((uint8_t*)"\0", 1);
    udp.write((uint8_t*)password.c_str(), password.length());
    
    udp.endPacket();
}
