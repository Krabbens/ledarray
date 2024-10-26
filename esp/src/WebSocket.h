#pragma once
#include <ESPAsyncWebServer.h>

#define SERVER_PORT 80
#define WS_URL "/ws"

class WebSocketServer {
public:
    WebSocketServer(uint16_t port = SERVER_PORT);
    ~WebSocketServer();

    void begin();
    void handleClient();
    void broadcastText(const String& message);
    void sendToClient(uint32_t clientId, const String& message);

private:
    uint16_t port;
    AsyncWebServer server;
    AsyncWebSocket ws;

    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                 AwsEventType type, void *arg, uint8_t *data, size_t len);
};

WebSocketServer::WebSocketServer(uint16_t port)
    : port(port), server(port), ws(WS_URL) {}

WebSocketServer::~WebSocketServer() {}

void WebSocketServer::begin() {
    ws.onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, 
                      AwsEventType type, void* arg, uint8_t* data, size_t len) {
        this->onEvent(server, client, type, arg, data, len);
    });
    
    server.addHandler(&ws);
    server.begin();
    Serial.println("WebSocket server started.");
}

void WebSocketServer::onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                              AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("Client connected: %u\n", client->id());
            break;

        case WS_EVT_DISCONNECT:
            Serial.printf("Client disconnected: %u\n", client->id());
            break;

        case WS_EVT_PONG:
            Serial.println("PONG received");
            break;

        case WS_EVT_ERROR:
            Serial.println("WebSocket error occurred");
            break;

        case WS_EVT_DATA:
            Serial.printf("Data received from client %u\n", client->id());
            Serial.printf("Payload: %s\n", (char*)data);
            break;
    }
}

void WebSocketServer::handleClient() {
    ws.cleanupClients();
}

void WebSocketServer::broadcastText(const String& message) {
    ws.textAll(message);
}

void WebSocketServer::sendToClient(uint32_t clientId, const String& message) {
    AsyncWebSocketClient* client = ws.client(clientId);
    if (client && client->status() == WS_CONNECTED) {
        client->text(message);
    } else {
        Serial.printf("Client %u not found or not connected\n", clientId);
    }
}

// void WebSocket::binData(byte* payload, unsigned int length) {
//     Debug::raw("WebSocket: Message arrived [");
//     Debug::raw("] ");
    
//     if (length < sizeof(Frame)) {
//         Debug::raw("Error: Payload too short to contain a Frame " + String(length) + "/" + String(sizeof(Frame)) + "\n");
//         return;
//     }
    
//     Frame* frame = (Frame*)payload;
//     unsigned int payloadLength = frame->content_length;

//     switch (frame->type) {
//         case check_alive:
//             {
//                 Debug::raw("Frame type: check_alive\n");
//                 this->sendInteger(alive_status, (int)currentState);

//                 break;
//             }
//         case alive_status:
//             Debug::raw("Frame type: alive_status\n");

//             break;
//         case ready:
//             Debug::raw("Frame type: ready\n");

//             break;
//         case buffer_size:
//             Debug::raw("Frame type: buffer_size\n");

//             break;
//         case animation_add:
//             {
//                 Debug::raw("Frame type: animation_add\n");
//                 const char* name = (char*)(payload + sizeof(Frame));
//                 const byte* animation = findAnimation((byte*)name, payloadLength);
//                 size_t animationLength = payloadLength - (animation - (byte*)name);
//                 if(!animDB->addAnimation(name, animation, animationLength)){
//                     Debug::error("Failed adding animation\n");
//                 }
//                 sendAnimationNames();
//                 break;
//             }
//         case animation_remove:
//             {
//                 Debug::raw("Frame type: animation_remove\n");
//                 const char* name = (char*)(payload + sizeof(Frame));
//                 Debug::info(name);
//                 if(!animDB->removeAnimation(name)){
//                     Debug::error("Failed removing animation\n");
//                 }
//                 sendAnimationNames();
//                 break;
//             }
//         case animation_get:
//             { 
//                 Debug::raw("Frame type: animation_get\n");
//                 sendAnimationNames();
//                 break;
//             }

//         case animation_play:
//             {
//                 Debug::raw("Frame type: animation_play\n");
//                 // Add logic for handling animation_play here
//                 const char* name = (char*)(payload + sizeof(Frame));
//                 Debug::info(name);
//                 size_t len = animDB->getAnimationSize(name);
//                 if(len == 0){
//                     Debug::info("Animation name not found");
//                 }
//                 else{
//                     byte* data = (byte*)ps_malloc(len);
//                     if(animDB->getAnimation(name, data, len)){
//                         delete ledArray;
//                         ledArray = new LedArray(data, len);
//                     }
//                     nextState = State::ANIMATION;
//                 }

//                 break;
//             }

//         case animation_clear:
//             {
//                 Debug::raw("Frame type: animation_clear\n");
            
//                 animDB->clear();

//                 break;
//             }
//         case get_size:
//             {
//                 Debug::raw("Frame type: get_size\n");

//                 SizeInfo sizeInfo = animDB->getSizeInfo();
//                 this->sendSizeInfo();

//                 break;
//             }
//         case animation_stop:
//             {
//                 Debug::raw("Frame type: animation_stop\n");

//                 nextState = State::STOP_ANIMATION;

//                 break;
//             }
//         default:
//             Debug::raw("Unknown Frame type\n");
//             break;
//     }
// }