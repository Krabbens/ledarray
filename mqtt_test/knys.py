import paho.mqtt.client as mqtt
import os
import struct
import time

# Definicje typów ramek
FRAME_TYPE_ANIMATION = 0
FRAME_TYPE_CHECK_ALIVE = 1
FRAME_TYPE_ALIVE_STATUS = 2
FRAME_TYPE_READY = 3
FRAME_TYPE_BUFFER_SIZE = 4

class FileChunkReader:
    def __init__(self, file, chunk_size):

        self.file = file
        self.chunk_size = chunk_size
        self.file_size = os.path.getsize(file.name)
        self.current_position = 0

    def get_chunk(self):
        chunk = b''
        while len(chunk) < self.chunk_size:
            self.file.seek(self.current_position)
            remaining_bytes = self.chunk_size - len(chunk)
            data = self.file.read(remaining_bytes)
            chunk += data
            
            if len(data) < remaining_bytes:
                self.current_position = 0  # Wróć na początek pliku
            else:
                self.current_position = (self.current_position + remaining_bytes) % self.file_size
        
        return chunk

file = open('image.dat', 'rb')
reader = FileChunkReader(file, 0)

# Funkcja do tworzenia ramki
def create_frame(frame_type, content):
    if isinstance(content, str):
        content_bytes = content.encode('utf-8')
    elif isinstance(content, int):
        content_bytes = struct.pack('I', content)
    else:
        content_bytes = content
    
    content_length = len(content_bytes)
    
    # Serializacja ramki (type jako 4-bajtowy int, content_length jako 4-bajtowy int)
    frame = struct.pack('I I', frame_type, content_length)
    return frame + content_bytes

# Funkcja do odczytywania ramki
def read_frame(payload):
    HEADER_SIZE = 8
    if len(payload) < HEADER_SIZE:
        raise ValueError("Payload is too short to contain a valid frame header")

    # Odczytujemy typ ramki i długość zawartości
    frame_type, content_length = struct.unpack('I I', payload[:HEADER_SIZE])

    print("Content length: " + str(content_length))

    if len(payload) < HEADER_SIZE + content_length:
        raise ValueError("Payload is shorter than expected based on content length")

    # Odczytujemy zawartość wiadomości
    content_bytes = payload[HEADER_SIZE:HEADER_SIZE + content_length]
    print(content_bytes)
    
    if frame_type in (FRAME_TYPE_CHECK_ALIVE, 
                      FRAME_TYPE_ALIVE_STATUS,
                      FRAME_TYPE_BUFFER_SIZE):
        if len(content_bytes) >= 4:
            content = struct.unpack('I', content_bytes[:4])[0]
        else:
            raise ValueError("Content length is not sufficient to unpack an integer")
    else:
        content = content_bytes.decode('utf-8')

    return frame_type, content

def on_connect(client, userdata, flags, rc, props):
    print("Connected with result code " + str(rc))
    client.subscribe("external")

def on_message(client, userdata, msg):
    frame_type, content = read_frame(msg.payload)
    
    # Działanie w zależności od typu ramki
    if frame_type == FRAME_TYPE_CHECK_ALIVE:
        print(f"Received CHECK_ALIVE message with integer content: " + str(content))
        client.publish("esp32/alive_status", 1)
    elif frame_type == FRAME_TYPE_ANIMATION:
        print("Received ANIMATION message with content: " + content)
    elif frame_type == FRAME_TYPE_ALIVE_STATUS:
        print("Received ALIVE_STATUS message with content: " + str(content))
    elif frame_type == FRAME_TYPE_READY:
        print(f"Received READY message with integer content: " + content)
        if(reader.chunk_size == 0):
            print("chunk size not set")
            frame_type = FRAME_TYPE_CHECK_ALIVE
            content = 42  # Przykładowa liczba całkowita do wysłania
            frame = create_frame(frame_type, content)
            client.publish("upper_esp", frame)
            print("Sent")
        else:
            chunk = reader.get_chunk()
            frame = create_frame(FRAME_TYPE_ANIMATION, chunk)
            print(len(chunk))
            client.publish("upper_esp", frame)
    elif frame_type == FRAME_TYPE_BUFFER_SIZE:
        print(f"Received BUFFER_SIZE message with integer content: " + str(content))
        reader.chunk_size = content
    else:
        print("Unknown frame type received")



client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.tls_set()
client.username_pw_set("esp", "_ledarray_nugget")

client.on_connect = on_connect
client.on_message = on_message

client.connect("5686adbdc3644dca8e63a851e72c3b21.s1.eu.hivemq.cloud", 8883, 60)

client.loop_forever()