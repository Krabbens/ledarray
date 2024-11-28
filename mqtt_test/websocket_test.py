import websocket
import threading
import struct
import time

# Typy ramek
FRAME_TYPE_ANIMATION_ADD = 5
FRAME_TYPE_ANIMATION_REMOVE = 6
FRAME_TYPE_ANIMATION_GET = 7
FRAME_TYPE_ANIMATION_PLAY = 8
FRAME_TYPE_ANIMATION_CLEAR = 9
FRAME_TYPE_ANIMATION_NAMES = 10

# Adres mDNS serwera WebSocket (zamień na odpowiedni dla Twojego ESP32)
SERVER_MDNS = "ws://upper_esp.local/ws"

# Obsługa zdarzeń WebSocket
def on_open(ws):
    print("Połączono z serwerem WebSocket.")

def on_close(ws, close_status_code, close_msg):
    print(f"Połączenie zamknięte: {close_status_code}, {close_msg}")

def on_message(ws, message):
    print(f"Odebrano wiadomość tekstową: {message}")

def on_data(ws, data, opcode, fin):
    if opcode == 0x9:  # Ping
        print("Odebrano PING od serwera.")
        ws.send("", opcode=0xA)  # Wysyła PONG w odpowiedzi
    elif opcode == 0xA:  # Pong
        print("Odebrano PONG.")
    else:
        handle_binary_frame(data)

def on_error(ws, error):
    print(f"Błąd WebSocket: {error}")

# Funkcja obsługująca dane binarne
def handle_binary_frame(data):
    if len(data) < struct.calcsize('<I I'):
        print("Odebrano niekompletną ramkę.")
        return

    frame_type, content_length = struct.unpack('<I I', data[:8])
    payload = data[8:]

    if len(payload) != content_length:
        print("Nieprawidłowa długość danych w ramce.")
        return

    print(f"Odebrano ramkę typu {frame_type} z danymi o długości {len(payload)}.")
    # Możesz dodać logikę obsługi różnych typów ramek tutaj

# Funkcja wysyłania ramki binarnej
def send_frame(ws, frame_type, payload=None):
    if payload is None:
        payload = b""
    content_length = len(payload)
    frame_header = struct.pack('<I I', frame_type, content_length)
    ws.send(frame_header + payload, opcode=0x2)
    print(f"Wysłano ramkę typu {frame_type} z danymi o długości {content_length}.")

# Interaktywne menu
def interactive_menu(ws):
    while True:
        print("\nMenu:")
        print("1. Add Animation")
        print("2. Remove Animation")
        print("3. Get Animation")
        print("4. Play Animation")
        print("5. Clear Animations")
        print("0. Exit")

        choice = input("Wybierz opcję: ")

        if choice == '0':
            ws.close()
            break
        elif choice == '1':
            animation_name = input("Podaj nazwę animacji: ").encode('utf-8') + b'\0'
            file_path = input("Podaj ścieżkę do pliku: ")
            try:
                with open(file_path, 'rb') as f:
                    file_data = f.read()
                payload = animation_name + file_data
                send_frame(ws, FRAME_TYPE_ANIMATION_ADD, payload)
            except FileNotFoundError:
                print("Nie znaleziono pliku.")
        elif choice == '2':
            animation_name = input("Podaj nazwę animacji do usunięcia: ").encode('utf-8') + b'\0'
            send_frame(ws, FRAME_TYPE_ANIMATION_REMOVE, animation_name)
        elif choice == '3':
            send_frame(ws, FRAME_TYPE_ANIMATION_GET)
        elif choice == '4':
            animation_name = input("Podaj nazwę animacji do odtworzenia: ").encode('utf-8') + b'\0'
            send_frame(ws, FRAME_TYPE_ANIMATION_PLAY, animation_name)
        elif choice == '5':
            send_frame(ws, FRAME_TYPE_ANIMATION_CLEAR)
        else:
            print("Niepoprawny wybór.")

# Funkcja główna
def main():
    ws = websocket.WebSocketApp(
        SERVER_MDNS,
        on_open=on_open,
        on_message=on_message,
        on_data=on_data,
        on_close=on_close,
        on_error=on_error
    )

    # Uruchomienie wątku WebSocket
    thread = threading.Thread(target=ws.run_forever, daemon=True)
    thread.start()

    try:
        interactive_menu(ws)
    except KeyboardInterrupt:
        print("Zamykam aplikację...")
        ws.close()

if __name__ == "__main__":
    main()
