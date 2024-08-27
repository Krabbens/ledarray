# Kolorowy wyświetlacz widmowy

## Spis Treści

- [Aplikacja](#flutter)
- [ESP PlatformIO](#esp-platformio)
- [PCB](#pcb)
- [Blender](#blender)
- [Przetwarzanie wstępne](#przetwarzanie-wstępne)

## Aplikacja do kontroli

1. Zainstaluj Flutter:

    - Pobierz Flutter SDK z https://flutter.dev/docs/get-started/install
    - Dodaj Flutter bin do PATH

2. Sprawdź instalację:

    ```bash
    flutter doctor
    ```

3. Uruchom projekt:

    ```bash
    cd /ścieżka/do/projektu
    flutter run
    ```

## ESP PlatformIO

1. Zainstaluj PlatformIO:

    - Pobierz PlatformIO IDE z https://platformio.org/install/ide?install=vscode

2. Skopiuj pliki z folderu esp/core_lib do odpowiadających folderów bibliotek espressif32 w domyślnej lokalizacji platformio

3. Uruchom projekt:

    - Otwórz PlatformIO IDE, załaduj projekt, użyj opcji "Build" i "Upload"

## PCB

1. Zainstaluj KiCad:

    - Pobierz KiCad z https://kicad.org/download/

2. Otwórz projekt:

    - Załaduj istniejący plik PCB w KiCad

## Blender

1. Zainstaluj Blender:

    - Pobierz Blender z https://www.blender.org/download/

2. Otwórz projekt:

    - Plik `cude.blend` jest głównym plikiem do renderowania animacji.
    - Plik `simul.blend` służy do symulacji animacji.

## Przetwarzanie wstępne

Zestaw skryptów służący do przetwarzania danych z blendera na format rozumiany przez program kontrolujący diody LED.