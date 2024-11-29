#pragma once

#include <WiFi.h>
#include <Preferences.h>
#include <WiFiUdp.h>
#include "Debug.h"

class PriorityWireless {
public:
    PriorityWireless(const String& prioritySSID);
    ~PriorityWireless();

    void begin();         // Inicjalizacja i rozpoczęcie działania
    bool isConnected();   // Sprawdzanie statusu połączenia

private:
    Preferences preferences;
    WiFiUDP udp;          // UDP do odbioru broadcastów
    String prioritySSID;  // Nazwa sieci priorytetowej
    TaskHandle_t monitorTaskHandle = nullptr;
    bool priorityConnected = false;

    bool connectToPriorityNetwork();
    bool connectToSavedNetwork();
    bool attemptConnection(const String& ssid, const String& password = "");

    void monitorNetworks();
    static void monitorTask(void* parameter);
    void receiveBroadcast();
    void saveCredentials(const String& ssid, const String& password);
};

PriorityWireless::PriorityWireless(const String& prioritySSID)
    : prioritySSID(prioritySSID) {
    Debug::info("PriorityWireless object created.");
    preferences.begin("wireless", false);
}

PriorityWireless::~PriorityWireless() {
    Debug::info("PriorityWireless object destroyed.");
    preferences.end();
    if (monitorTaskHandle) {
        vTaskDelete(monitorTaskHandle);
    }
}

void PriorityWireless::begin() {
    Debug::info("Starting PriorityWireless...");

    if (connectToSavedNetwork()) {
        Debug::info("Connected to saved network.");
    }

    if (!monitorTaskHandle) {
        xTaskCreatePinnedToCore(monitorTask, "MonitorNetworks", 4096, this, 1, &monitorTaskHandle, 1);
    }
}

bool PriorityWireless::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool PriorityWireless::connectToPriorityNetwork() {
    Debug::info("Attempting to connect to priority network: " + prioritySSID);
    priorityConnected = attemptConnection(prioritySSID);
    return priorityConnected; // Łączy się bez hasła
}

bool PriorityWireless::connectToSavedNetwork() {
    if (!preferences.isKey("ssid")) {
        Debug::error("No saved WiFi credentials.");
        return false;
    }

    String ssid = preferences.getString("ssid");
    String password = preferences.getString("password");

    Debug::info("Attempting to connect to saved network: " + ssid);
    bool returnValue = attemptConnection(ssid, password);
    priorityConnected = returnValue ? false : priorityConnected;
    return priorityConnected;
}

bool PriorityWireless::attemptConnection(const String& ssid, const String& password) {
    Debug::info("Connecting to SSID: " + ssid);

    WiFi.begin(ssid.c_str(), password.isEmpty() ? nullptr : password.c_str());
    int retries = 0;

    while (WiFi.status() != WL_CONNECTED && retries < 3) {
        delay(1000);
        Debug::info("Connecting... attempt " + String(retries + 1));
        retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Debug::info("Connected to " + ssid + " with IP: " + WiFi.localIP().toString());
        return true;
    } else {
        Debug::error("Failed to connect to " + ssid);
        return false;
    }
}

void PriorityWireless::monitorNetworks() {
    while (true) {
        if(!priorityConnected){
            Debug::info("Scanning for priority network...");
            int networkCount = WiFi.scanNetworks(); // Skanowanie sieci

            for (int i = 0; i < networkCount; i++) {
                if (WiFi.SSID(i) == prioritySSID) {
                    Debug::info("Priority network found. Connecting...");
                    if (connectToPriorityNetwork()) {
                        Debug::info("Switched to priority network.");
                        priorityConnected = true;
                    }
                    break; // Przerywamy pętlę po przełączeniu na sieć priorytetową
                }
            }
        }
        else{
            receiveBroadcast();
        }
        delay(1000);
    }
}

void PriorityWireless::monitorTask(void* parameter) {
    PriorityWireless* instance = static_cast<PriorityWireless*>(parameter);
    instance->monitorNetworks();
}

void PriorityWireless::receiveBroadcast() {
    Debug::info("Receiving broadcast...");
    int packetSize = udp.parsePacket();  // Sprawdzamy, czy nadeszły dane
    delay(1000);
    Debug::raw(packetSize);
    if (packetSize) {
        char packetBuffer[255];  // Bufor na odebrany pakiet
        int len = udp.read(packetBuffer, 255);  // Odczytanie pakietu

        if (len > 0) {
            String message(packetBuffer);
            int nullIndex = message.indexOf('\0');  // Znalazłem separator
            if (nullIndex != -1) {
                String ssid = message.substring(0, nullIndex);  // SSID przed separator
                String password = message.substring(nullIndex + 1);  // Hasło po separatorze

                // Zapisujemy dane i próbujemy połączyć
                saveCredentials(ssid, password);
                connectToPriorityNetwork();
            }
        }
    }
}

void PriorityWireless::saveCredentials(const String& ssid, const String& password) {
    Debug::info("Saving new credentials: SSID = " + ssid + ", Password = " + password);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
}
