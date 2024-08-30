import paho.mqtt.client as mqtt
import struct
import os

# Definicja typów ramek
FRAME_TYPE_ANIMATION_ADD = 5
FRAME_TYPE_ANIMATION_REMOVE = 6
FRAME_TYPE_ANIMATION_GET = 7
FRAME_TYPE_ANIMATION_PLAY = 8
FRAME_TYPE_ANIMATION_CLEAR = 9
FRAME_TYPE_ANIMATION_NAMES = 10

# Ustawienia MQTT
MQTT_BROKER = '5686adbdc3644dca8e63a851e72c3b21.s1.eu.hivemq.cloud'
MQTT_PORT = 8883
MQTT_TOPIC = 'upper_esp'

# Klient MQTT
client = mqtt.Client()

def on_connect(client, userdata, flags, rc, esd):
    print(f'Connected with result code {rc}')
    # Połącz się z brokerem
    client.subscribe("external")

def on_message(client, userdata, msg):
    # Odbieranie wiadomości
    print(f"Received message on topic {msg.topic}")
    handle_message(msg.payload)

def handle_message(payload):
    # Rozpakowanie ramki
    if len(payload) < struct.calcsize('I I'):
        print("Payload too short")
        return

    frame_type, content_length = struct.unpack('I I', payload[:8])

    if len(payload) != content_length + 8:
        print("Content length mismatch")
        return

    data = payload[8:]
    print(f"Received frame type {frame_type} with data of length {len(data)}")

def send_frame(frame_type, data = None):
    if data is None:
        content_length = 0
        frame = struct.pack('I I', frame_type, content_length)
    else:
        content_length = len(data) + 1
        frame = struct.pack('I I', frame_type, content_length) + data + '\0'.encode('utf-8')
    client.publish(MQTT_TOPIC, frame)
    print(f"Sent frame type {frame_type} with data of length {content_length}")

def send_file(file_path, animation_name):
    # Wysyłanie pliku w kawałkach
    with open(file_path, 'rb') as f:
        file_data = f.read()

    # Przygotuj nazwę animacji zakończoną znakiem null
    name_with_null = (animation_name + '\0').encode('utf-8')

    # Przygotuj dane do wysłania: [nazwa_animacji + dane_pliku]
    data_to_send = name_with_null + file_data

    # Wysyłanie całej animacji jako jedna wiadomość
    send_frame(FRAME_TYPE_ANIMATION_ADD, data_to_send)

def interactive_menu():
    while True:
        print("\nMenu:")
        print("1. Add Animation")
        print("2. Remove Animation")
        print("3. Get Animation")
        print("4. Play Animation")
        print("5. Clear Animations")
        print("0. Exit")
        
        choice = input("Choose an option: ")
        
        if choice == '0':
            break
        
        if choice == '1':
            #file_path = input("Enter file path to add animation: ")
            #send_file(file_path)
            animation_name = input("Enter animation name: ")
            send_file("diff.dat", animation_name)
        
        elif choice == '2':
            name = input("Enter animation name to remove: ").encode('utf-8')
            send_frame(FRAME_TYPE_ANIMATION_REMOVE, name)
        
        elif choice == '3':
            send_frame(FRAME_TYPE_ANIMATION_GET)
        
        elif choice == '4':
            name = input("Enter animation name to play: ").encode('utf-8')
            send_frame(FRAME_TYPE_ANIMATION_PLAY, name)
        
        elif choice == '5':
            send_frame(FRAME_TYPE_ANIMATION_CLEAR, b'')
        
        else:
            print("Invalid choice, please try again.")

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.tls_set()
client.username_pw_set("esp", "_ledarray_nugget")

# Rejestracja funkcji callback
client.on_connect = on_connect
client.on_message = on_message

# Połączenie z brokerem
client.connect("5686adbdc3644dca8e63a851e72c3b21.s1.eu.hivemq.cloud", 8883, 60)

# Rozpoczęcie pętli MQTT
client.loop_start()

# Uruchomienie menu interaktywnego
interactive_menu()

# Zatrzymanie pętli MQTT i rozłączenie klienta
client.loop_stop()
client.disconnect()
