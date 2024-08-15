import paho.mqtt.client as mqtt
import struct

# Definicje typów ramek
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

# Funkcja do odczytywania ramki
def read_frame(payload):
   # Ustalamy minimalną długość ramki (4 bajty na typ + 4 bajty na długość)
    HEADER_SIZE = 8
    if len(payload) < HEADER_SIZE:
        print(len(payload))
        raise ValueError("Payload is too short to contain a valid frame header")

    # Odczytujemy typ ramki i długość zawartości
    frame_type, content_length = struct.unpack('I I', payload[:HEADER_SIZE])
    # Upewniamy się, że długość zawartości jest prawidłowa
    if len(payload) < HEADER_SIZE + content_length:
        raise ValueError("Payload is shorter than expected based on content length")

    # Odczytujemy zawartość wiadomości
    content = payload[HEADER_SIZE:HEADER_SIZE + content_length].decode('utf-8')
    return frame_type, content

def on_connect(client, userdata, flags, rc, props):
    print("Connected with result code " + str(rc))
    client.subscribe("external")

def on_message(client, userdata, msg):
    frame_type, content = read_frame(msg.payload)
    
    # Działanie w zależności od typu ramki
    if frame_type == FRAME_TYPE_CHECK_ALIVE:
        print("Received CHECK_ALIVE message with content: " + content)
        client.publish("esp32/alive_status", 1)
    elif frame_type == FRAME_TYPE_ANIMATION:
        print("Received ANIMATION message with content: " + content)
    elif frame_type == FRAME_TYPE_ALIVE_STATUS:
        print("Received ALIVE_STATUS message with content: " + content)
    elif frame_type == FRAME_TYPE_READY:
        print("Received READY message with content: " + content)
    else:
        print("Unknown frame type received")

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.tls_set()
client.username_pw_set("esp", "_ledarray_nugget")

client.on_connect = on_connect
client.on_message = on_message

client.connect("5686adbdc3644dca8e63a851e72c3b21.s1.eu.hivemq.cloud", 8883, 60)

# Tworzenie ramki do wysłania
frame_type = FRAME_TYPE_CHECK_ALIVE
content = "yo"  # Przykładowa zawartość wiadomości
frame = create_frame(frame_type, content)

# Wysyłanie wiadomości MQTT
client.publish("upper_esp", frame)
print("Sent")

client.loop_forever()