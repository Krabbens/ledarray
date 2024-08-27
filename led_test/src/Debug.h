#pragma once

#include <Arduino.h>
#include <Preferences.h>

#define D_LOG

class Debug {
public:
    static void init();
    static void log(String message);
    static void error(String message);
    static void warn(String message);
    static void info(String message);

    template <typename T>
    static void raw(T message) {
        #ifdef D_LOG
        Serial.print(message);
        #endif
    }
};

void Debug::init() {
    #ifdef D_LOG
    Serial.begin(115200);
    delay(4000);
    #endif
}

void Debug::log(String message) {
    #ifdef D_LOG
    Serial.println(message);
    #endif
}

void Debug::error(String message) {
    #ifdef D_LOG
    Serial.println("ERROR: " + message);
    #endif
}

void Debug::warn(String message) {
    #ifdef D_LOG
    Serial.println("WARN: " + message);
    #endif
}

void Debug::info(String message) {
    #ifdef D_LOG
    Serial.println("INFO: " + message);
    #endif
}