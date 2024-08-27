#pragma once
#include "SPIFFS.h"
#include "FS.h"
#include "Debug.h"

class FileManager {
public:
    static bool begin();
    static bool writeFile(const char* path, const byte* data, size_t length);
    static bool readFile(const char* path, byte* buffer, size_t length);
    static bool removeFile(const char* path);
    static void removeAllFiles();
    static size_t getFreeSpace();
};

bool FileManager::begin(){
    if (!SPIFFS.begin(true)) {
        Debug::info("Failed to mount SPIFFS. Formatting now...");
        if (SPIFFS.format()) {
            Debug::info("SPIFFS formatted successfully.");
            if (SPIFFS.begin()) {
                Debug::info("SPIFFS mounted successfully after format.");
                return true;
            } else {
                Debug::info("Failed to mount SPIFFS after formatting.");
                return false;
            }
        } 
        else {
            Debug::info("SPIFFS formatting failed.");
            return false;
        }
    } 
    else {
        Debug::info("SPIFFS mounted successfully.");
        return true;
    }
}

bool FileManager::writeFile(const char* path, const byte* data, size_t length) {
    File file = SPIFFS.open(path, FILE_WRITE);
    if (!file) {
        Debug::error("Couldn't open file: " + String(path) +"\n");
        return false;
    }
    file.write(data, length);
    file.close();
    Debug::info("Successfully saved file: " + String(path));
    return true;
}

bool FileManager::readFile(const char* path, byte* buffer, size_t length) {
    File file = SPIFFS.open(path, FILE_READ);
    if (!file) {
        Debug::error("Couldn't read file: " + String(path) +"\n");
        return "";
    }
    size_t bytesRead = file.read(buffer, length);
    file.close();

    if (bytesRead != length) {
        Debug::error("Read only" + String(bytesRead) + "/" + String(length) + " bytes from file: " + String(path) +"\n");
    }

    Debug::info("Successfully read file: " + String(path));
    return true;
}

bool FileManager::removeFile(const char* path) {
    if (SPIFFS.remove(path)) {
        Debug::info("Successfully deleted file: " + String(path));
        return true;
    } else {
        Debug::error("Couldn't delete file: " + String(path) +"\n");
        return false;
    }
}

void FileManager::removeAllFiles() {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
        const char* path = file.path();
        removeFile(path); 
        file = root.openNextFile();
    }
    Debug::info("Successfully deleted all files\n");
}

size_t FileManager::getFreeSpace() {
    size_t totalBytes = SPIFFS.totalBytes();
    size_t usedBytes = SPIFFS.usedBytes();

    Debug::info("[File Manager]");
    Debug::raw("\tTotal: " + String(totalBytes) + "B");
    Debug::raw("\tUsed: " + String(usedBytes) + "B");
    Debug::raw("\tFree: " + String(totalBytes - usedBytes) + "B");
    Debug::raw("\n");

    return totalBytes - usedBytes;
}
