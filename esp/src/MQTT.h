#pragma once
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "Debug.h"
#include "Frame.h"
#include "LedArray.h"
#include "AnimDB.h"

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

class MQTT {
public:
    MQTT(WiFiClientSecure* wifiClient);
    ~MQTT();

    void connectToBroker();
    void publish(const char* topic, const char* payload);
    void publish(const char* topic, byte* payload, unsigned int len);
    void publishInteger(const char* topic, FrameType type, int value);
    void publishSizeInfo(const char* topic, FrameType type, SizeInfo sizeInfo);
    void subscribe(const char* topic);
    void loop() {
        client.loop();
    }
private:
    PubSubClient client;
    const char *msg_broker, *msg_port, *msg_username, *msg_password;
    static void callback(char* topic, byte* payload, unsigned int length);
};

MQTT::MQTT(WiFiClientSecure* wifiClient) : client(*wifiClient) {
    Debug::info("MQTT object created.");
    msg_broker = "5686adbdc3644dca8e63a851e72c3b21.s1.eu.hivemq.cloud";
    msg_port = "8883";
    msg_username = "esp";
    msg_password = "_ledarray_nugget";
    wifiClient->setCACert(root_ca);
    client.setServer(msg_broker, atoi(msg_port));

    client.setCallback(callback);
}

MQTT::~MQTT() {
    Debug::info("MQTT object destroyed.");
}

void MQTT::connectToBroker() {
    Debug::info("Connecting to broker...");
    while (!client.connected()) {
        Debug::info("Attempting MQTT connection...");
        if (client.connect("ESP32Client_Top", msg_username, msg_password)) {
            Debug::info("Connected to broker.");
        }
        else {
            Debug::info("Failed to connect to broker.");
            delay(5000);
        }
    }
}

void MQTT::publish(const char* topic, const char* payload) {
    if (client.connected()) {
        client.publish(topic, payload);
        Debug::info(payload);
    }
}

void MQTT::publish(const char* topic, byte* payload, unsigned int len){
    if (client.connected()) {
        client.publish(topic, payload, len);
    }
}

void MQTT::publishInteger(const char* topic, FrameType type, int value){
    Frame frame;
    frame.type = type;
    frame.content_length = sizeof(int);          
    byte buffer[sizeof(frame) + sizeof(int)];
    memcpy(buffer, &frame, sizeof(frame));
    memcpy(buffer + sizeof(frame), &value, sizeof(int));
    publish(topic, buffer, sizeof(buffer));
}

void MQTT::publishSizeInfo(const char* topic, FrameType type, SizeInfo sizeInfo){
    Frame frame;
    frame.type = type;
    frame.content_length = sizeof(SizeInfo);          
    byte buffer[sizeof(frame) + sizeof(SizeInfo)];
    memcpy(buffer, &frame, sizeof(frame));
    memcpy(buffer + sizeof(frame), &sizeInfo, sizeof(SizeInfo));
    publish(topic, buffer, sizeof(buffer));
}

void MQTT::subscribe(const char* topic) {
    if (client.connected()) {
        client.subscribe(topic);
    }
}

extern MQTT* mqtt;
extern LedArray *ledArray;
extern AnimDB *animDB;

const byte* findAnimation(const byte* data, size_t len) {
    size_t nameLen = strlen((const char *)data);

    if (nameLen >= len) {
        return NULL; 
    }
    return data + nameLen + 1;
}

void sendAnimationNames(){
    size_t namesLen = 100;
    char* pld = (char*)malloc(sizeof(char) * namesLen + sizeof(Frame));
    char* names = pld + sizeof(Frame);
    if(!animDB->getAllAnimationNames(names, namesLen)){
        Debug::error("Failed reading animation names\n");
    }
    Frame namesFrame;
    namesFrame.type = animation_names;
    namesFrame.content_length = strlen(names) + 1;
    Debug::info(String(namesFrame.content_length));
                                                
    memcpy(pld, &namesFrame, sizeof(namesFrame));

    mqtt->publish("external", (byte*)pld, namesFrame.content_length + sizeof(Frame));
    free(pld);

    SizeInfo sizeInfo = animDB->getSizeInfo();
    mqtt->publishSizeInfo("external", info_size, sizeInfo);
}

void MQTT::callback(char* topic, byte* payload, unsigned int length) {
    Debug::raw("MQTT: Message arrived [");
    Debug::raw(topic);
    Debug::raw("] ");
    
    if (length < sizeof(Frame)) {
        Debug::raw("Error: Payload too short to contain a Frame " + String(length) + "/" + String(sizeof(Frame)) + "\n");
        return;
    }
    
    Frame* frame = (Frame*)payload;
    unsigned int payloadLength = frame->content_length;

    switch (frame->type) {
        case animation:
            {
                // Debug::raw("Frame type: animation, len: " + String(length) + ", len - frame: " + String(length - sizeof(Frame)) + "\n");
                // if(length >= sizeof(Frame) + sizeof(CRGB) * ALL_LEDS * SEC_IN_BUFFER * FRAMES_PER_SEC){
                //     ledArray->fillBuffer((CRGB*)(payload+sizeof(Frame)));
                // }
                // else{
                //     Debug::error("Payload too short\n");
                // }
                break;
            }
        case check_alive:
            {
                Debug::raw("Frame type: check_alive\n");
                mqtt->publishInteger("external", alive_status, 1);

                break;
            }
        case alive_status:
            Debug::raw("Frame type: alive_status\n");

            break;
        case ready:
            Debug::raw("Frame type: ready\n");

            break;
        case buffer_size:
            Debug::raw("Frame type: buffer_size\n");

            break;
        case animation_add:
            {
                Debug::raw("Frame type: animation_add\n");
                const char* name = (char*)(payload + sizeof(Frame));
                const byte* animation = findAnimation((byte*)name, payloadLength);
                size_t animationLength = payloadLength - (animation - (byte*)name);
                if(!animDB->addAnimation(name, animation, animationLength)){
                    Debug::error("Failed adding animation\n");
                }
                sendAnimationNames();
                break;
            }
        case animation_remove:
            {
                Debug::raw("Frame type: animation_remove\n");
                const char* name = (char*)(payload + sizeof(Frame));
                Debug::info(name);
                if(!animDB->removeAnimation(name)){
                    Debug::error("Failed removing animation\n");
                }
                sendAnimationNames();
                break;
            }
        case animation_get:
            { 
                Debug::raw("Frame type: animation_get\n");
                sendAnimationNames();
                break;
            }

        case animation_play:
            {
                Debug::raw("Frame type: animation_play\n");
                // Add logic for handling animation_play here
                const char* name = (char*)(payload + sizeof(Frame));
                Debug::info(name);
                size_t len = animDB->getAnimationSize(name);
                if(len == 0){
                    Debug::info("Animation name not found");
                }
                else{
                    byte* data = (byte*)ps_malloc(len);
                    if(animDB->getAnimation(name, data, len)){
                        delete ledArray;
                        ledArray = new LedArray(data, len);
                    }
                }

                break;
            }

        case animation_clear:
            {
                Debug::raw("Frame type: animation_clear\n");
            
                animDB->clear();

                break;
            }
        case get_size:
            {
                SizeInfo sizeInfo = animDB->getSizeInfo();
                mqtt->publishSizeInfo("external", info_size, sizeInfo);

                break;
            }
        case animation_stop:
            {
                delete ledArray;
                ledArray = NULL;
            }
        default:
            Debug::raw("Unknown Frame type\n");
            break;
    }
}