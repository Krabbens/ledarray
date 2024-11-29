#pragma once
#include <WebSocketsServer.h>
#include <cstring>
#include "AnimDB.h"

#define SERVER_PORT 80
#define WS_URL "/ws"

#define NAMES_ARR_SIZE 100

class WebSocketServer {
public:
    WebSocketServer(uint16_t port = SERVER_PORT);
    ~WebSocketServer();

    void begin();
    void loop();
    void broadcastText(const String& message);
    void sendToClient(uint8_t clientId, const String& message);
    void sendToClient(uint8_t clientId, const byte* data, size_t length);

private:
    uint16_t port;
    WebSocketsServer ws;
    char* namesBuffer;

    void onEvent(uint8_t clientId, WStype_t type, uint8_t* payload, size_t length);
    void binData(uint8_t clientId, byte* payload, unsigned int length);

    void handleAnimationAdd(uint8_t clientId, byte* payload, unsigned int payloadLength);
    void handleAnimationRemove(uint8_t clientId, byte* payload, unsigned int payloadLength);
    void handleAnimationGet(uint8_t clientId, byte* payload, unsigned int payloadLength);
    void handleAnimationPlay(uint8_t clientId, byte* payload, unsigned int payloadLength);
    void handleAnimationClear(uint8_t clientId, byte* payload, unsigned int payloadLength);
    void handleAnimationStop(uint8_t clientId, byte* payload, unsigned int payloadLength);
    void handleGetSize(uint8_t clientId, byte* payload, unsigned int payloadLength);

    const byte* findAnimation(const byte* data, size_t len);
    void sendSizeInfo(uint8_t clientId, SizeInfo sizeInfo);
    void sendAnimationNames(uint8_t clientId);
};

WebSocketServer::WebSocketServer(uint16_t port)
    : port(port), ws(port), namesBuffer(NULL) {}

WebSocketServer::~WebSocketServer() {
    if (namesBuffer != NULL) {
        free(namesBuffer);
    }
}

void WebSocketServer::begin() {
    ws.begin();
    ws.onEvent([this](uint8_t clientId, WStype_t type, uint8_t* payload, size_t length) {
        this->onEvent(clientId, type, payload, length);
    });
    Serial.println("WebSocket server started.");
}

void WebSocketServer::loop() {
    ws.loop();

    static unsigned long lastPingTime = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastPingTime > 5000) { // Co 5 sekund
        Serial.println("Sending ping to all clients...");
        ws.broadcastPing();
        lastPingTime = currentTime;
    }
}

void WebSocketServer::sendToClient(uint8_t clientId, const byte* data, size_t length) {
    ws.sendBIN(clientId, data, length);
}

void WebSocketServer::onEvent(uint8_t clientId, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            Serial.printf("Client connected: %u\n", clientId);
            break;

        case WStype_DISCONNECTED:
            Serial.printf("Client disconnected: %u\n", clientId);
            break;

        case WStype_TEXT:
            Serial.printf("Text message received from client %u\n", clientId);
            break;

        case WStype_BIN:
            Serial.printf("Binary message received from client %u\n", clientId);
            binData(clientId, payload, length);
            break;

        case WStype_ERROR:
            Serial.printf("WebSocket error occurred on client %u\n", clientId);
            break;

        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_PING:
        case WStype_PONG:
            Serial.printf("PONG reveived from client: %u\n", clientId);
            break;
    }
}

void WebSocketServer::binData(uint8_t clientId, byte* payload, unsigned int length) {
    if (length < sizeof(Frame)) {
        Debug::raw("Error: Payload too short to contain a Frame\n");
        return;
    }

    Frame* frame = (Frame*)payload;
    unsigned int payloadLength = frame->content_length;

    switch (frame->type) {
        case animation_add:
            handleAnimationAdd(clientId, payload, payloadLength);
            break;

        case animation_remove:
            handleAnimationRemove(clientId, payload, payloadLength);
            break;

        case animation_get:
            handleAnimationGet(clientId, payload, payloadLength);
            break;

        case animation_play:
            handleAnimationPlay(clientId, payload, payloadLength);
            break;

        case animation_clear:
            handleAnimationClear(clientId, payload, payloadLength);
            break;

        case animation_stop:
            handleAnimationStop(clientId, payload, payloadLength);
            break;

        case get_size:
            handleGetSize(clientId, payload, payloadLength);
            break;

        default:
            Debug::raw("Unknown Frame type\n");
            break;
    }
}

extern LedArray *ledArray;
extern AnimDB *animDB;
extern State currentState;
extern State nextState;

const byte* WebSocketServer::findAnimation(const byte* data, size_t len) {
    size_t nameLen = strlen((const char *)data);

    if (nameLen >= len) {
        return NULL;  // Return NULL if the animation name exceeds length
    }
    return data + nameLen + 1;
}

void WebSocketServer::sendSizeInfo(uint8_t client, SizeInfo sizeInfo) {
    Frame frame;
    frame.type = FrameType::info_size;
    frame.content_length = sizeof(SizeInfo);          
    byte buffer[sizeof(frame) + sizeof(SizeInfo)];
    memcpy(buffer, &frame, sizeof(frame));
    memcpy(buffer + sizeof(frame), &sizeInfo, sizeof(SizeInfo));
    sendToClient(client, buffer, sizeof(buffer));
}

void WebSocketServer::sendAnimationNames(uint8_t client) {

    static size_t bufferLen = NAMES_ARR_SIZE + sizeof(Frame);

    if (namesBuffer == NULL) {
        namesBuffer = (char*)malloc(bufferLen);
        if (namesBuffer == NULL) {
            Debug::error("Failed to allocate memory for namesBuffer\n");
            return;
        }
    }
    
    size_t requiredSize = animDB->getAllAnimationNamesSize() + sizeof(Frame);

    if (requiredSize > bufferLen) {
        bufferLen = requiredSize * 1.5;
        namesBuffer = (char*)realloc(namesBuffer, bufferLen);
        
        if (namesBuffer == NULL) {
            Debug::error("Failed to reallocate memory for namesBuffer\n");
            return;
        }
    }

    char* names = namesBuffer + sizeof(Frame);
    size_t namesLen = bufferLen - sizeof(Frame);

    if (!animDB->getAllAnimationNames(names, namesLen)) {
        Debug::error("Failed reading animation names\n");
        return;
    }

    Frame namesFrame;
    namesFrame.type = animation_names;
    namesFrame.content_length = strlen(names);  // space for '\0'
    
    Debug::info(String(namesFrame.content_length));

    memcpy(namesBuffer, &namesFrame, sizeof(namesFrame));

    sendToClient(client, (byte*)namesBuffer, namesFrame.content_length + sizeof(Frame));
}

void WebSocketServer::handleAnimationAdd(uint8_t client, byte* payload, unsigned int payloadLength) {
    Debug::raw("Frame type: animation_add\n");
    const char* name = (char*)(payload + sizeof(Frame));
    const byte* animation = findAnimation((byte*)name, payloadLength);
    size_t animationLength = payloadLength - (animation - (byte*)name);
    
    if (!animDB->addAnimation(name, animation, animationLength)) {
        Debug::error("Failed adding animation\n");
    }
    sendAnimationNames(client);
}

void WebSocketServer::handleAnimationRemove(uint8_t client, byte* payload, unsigned int payloadLength) {
    Debug::raw("Frame type: animation_remove\n");
    const char* nameToRemove = (char*)(payload + sizeof(Frame));
    Debug::info(nameToRemove);
    
    if (!animDB->removeAnimation(nameToRemove)) {
        Debug::error("Failed removing animation\n");
    }
    sendAnimationNames(client);
}

void WebSocketServer::handleAnimationGet(uint8_t client, byte* payload, unsigned int payloadLength) {
    Debug::raw("Frame type: animation_get\n");
    if(ledArray == NULL)sendAnimationNames(client);
}

void WebSocketServer::handleAnimationPlay(uint8_t client, byte* payload, unsigned int payloadLength) {
    Debug::raw("Frame type: animation_play\n");
    const char* nameToPlay = (char*)(payload + sizeof(Frame));
    Debug::info(nameToPlay);
    size_t len = animDB->getAnimationSize(nameToPlay);
    
    if (len == 0) {
        Debug::info("Animation name not found");
    } else {
        byte* data = (byte*)ps_malloc(len);
        if (animDB->getAnimation(nameToPlay, data, len)) {
            delete ledArray;
            ledArray = new LedArray(data, len);
        }
        nextState = State::ANIMATION;
    }
}

void WebSocketServer::handleAnimationClear(uint8_t client, byte* payload, unsigned int payloadLength) {
    Debug::raw("Frame type: animation_clear\n");
    animDB->clear();
}

void WebSocketServer::handleAnimationStop(uint8_t client, byte* payload, unsigned int payloadLength) {
    Debug::raw("Frame type: animation_stop\n");
    nextState = State::STOP_ANIMATION;
}

void WebSocketServer::handleGetSize(uint8_t client, byte* payload, unsigned int payloadLength) {
    Debug::raw("Frame type: get_size\n");
    if(ledArray == NULL){
        SizeInfo sizeInfo = animDB->getSizeInfo();
        sendSizeInfo(client, sizeInfo);
    }
}