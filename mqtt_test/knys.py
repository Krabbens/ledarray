import paho.mqtt.client as mqtt
import struct

# Definicje typów ramek (muszą być zgodne z tymi w kodzie C++)
FRAME_TYPE_ANIMATION = 0
FRAME_TYPE_CHECK_ALIVE = 1
FRAME_TYPE_ALIVE_STATUS = 2
FRAME_TYPE_READY = 3

# Funkcja do tworzenia ramki
def create_frame(frame_type, content):
    content_bytes = content.encode('utf-8')  # Zakładamy, że zawartość jest tekstowa
    content_length = len(content_bytes)
    
    # Serializacja ramki (type jako 4-bajtowy int, content_length jako 4-bajtowy int)
    frame = struct.pack('I I', frame_type, content_length)
    return frame + content_bytes


def on_connect(client, userdata, flags, rc, props):
    print("Connected with result code " + str(rc))
    client.subscribe("esp32/check_alive")

def on_message(client, userdata, msg):
    client.publish("esp32/alive_status", 1)
    print(msg.topic + " " + str(msg.payload))

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.tls_set()
client.username_pw_set("esp", "_ledarray_nugget")

client.connect("5686adbdc3644dca8e63a851e72c3b21.s1.eu.hivemq.cloud", 8883, 60)

# Tworzenie ramki do wysłania
frame_type = FRAME_TYPE_CHECK_ALIVE
content = "yo"  # Przykładowa zawartość wiadomości
frame = create_frame(frame_type, content)

# Wysyłanie wiadomości MQTT
client.publish("upper_esp", frame)
print("sent")

client.loop()