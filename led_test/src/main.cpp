#include <Arduino.h>
#include "esp_timer.h"
#include "driver/spi_common.h"

#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_BUS FSPI

#define LED_CLOCK 1
#define LED_DATA 2
#define NUM_LEDS 40
// #define LED_DATA_RATE_MHZ 20

// #include <FastLED.h>
// CRGB leds[NUM_LEDS];
#include "apa102.h"

apa102_driver_t APA102;

static int docolors()
{
    for (int i = 0; i < LEDSTRIP.count; i++)
    {
        int v = (0xFF * ((i + LEDSTRIP.phase) % LEDSTRIP.count)) / LEDSTRIP.count;
        LEDSTRIP.leds[i] = RGBL(v, v + 0x44, v + 0x88, 0xFF);
    }
    return ESP_OK;
}

void setup()
{
    Serial.begin(115200);
    delay(5000);
    init_apa102_driver(&APA102);

    LEDSTRIP.init(40);
    LEDSTRIP.refresh = docolors;

    // FastLED.addLeds<SK9822, LED_DATA, LED_CLOCK, BGR, DATA_RATE_MHZ(LED_DATA_RATE_MHZ)>(&leds[0], NUM_LEDS);
    Serial.println("Hello there");
}

void loop()
{
    // fill_solid(leds, NUM_LEDS, CRGB::White);
    uint64_t start = esp_timer_get_time();

    LEDSTRIP.update();

    // FastLED.show();
    uint64_t end = esp_timer_get_time();

    Serial.printf("Speed: %d mhz, Sample: %llu\n", CONFIG_LMTZ_APA102_CLOCK_SPEED / 1000000, end - start);

}