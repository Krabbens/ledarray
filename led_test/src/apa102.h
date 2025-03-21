#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_system.h>
#include <esp_log.h>
#include <driver/spi_master.h>

#define CONFIG_LMTZ_APA102_NUM_LEDS 40
#define CONFIG_LMTZ_APA102_SPI_HSPI y
#define CONFIG_LMTZ_APA102_DMA_CHANNEL SPI_DMA_CH_AUTO
#define CONFIG_LMTZ_APA102_PIN_MOSI 7
#define CONFIG_LMTZ_APA102_PIN_SCLK 9
#define CONFIG_LMTZ_APA102_CLOCK_SPEED 60 * 1000 * 1000
#define CONFIG_LMTZ_APA102_SPI_HOST SPI2_HOST

typedef uint32_t apa102_color_t;

// every element is scaled from 0 to 0xFF
#define APA102_RGBL(r, g, b, lum)                                                                                      \
    ((((r) & 0xFF) << 24) | (((g) & 0xFF) << 16) | (((b) & 0xFF) << 8) | ((((lum) & 0xFF) >> 3)) | 0xE0)
#define APA102_RGB32(v) (\
	  ((v) << 8)  \
	|(((v) >> 24) \
	? ((v) >> 27) \
	: 0x0000001F) \
	| 0x000000E0;

#define RGBL APA102_RGBL
#define RGB32 APA102_RGB32

#if (CONFIG_LMTZ_APA102_SPI_HSPI)
#define CONFIG_LMTZ_APA102_SPI_HOST HSPI_HOST
#elif (CONFIG_LMTZ_APA102_SPI_VSPI)
#define CONFIG_LMTZ_APA102_SPI_HOST VSPI_HOST
#endif

typedef struct
{
    union {
        apa102_color_t *txbuffer;
        apa102_color_t *leds;
    };
    //[CONFIG_LMTZ_APA102_MAX_TRANSFER/sizeof(apa102_color_t)];

    int (*init)(int nleds);
    int (*deinit)();
    int (*refresh)();
    int (*update)();

    uint16_t phase;
    spi_host_device_t spi_host;
    int dma_channel;
    int count;
    spi_bus_config_t bus_config;
    spi_device_interface_config_t dev_config;
    spi_transaction_t transaction;
    spi_device_handle_t device;

} apa102_driver_t;

// typedef void (*apa102_refresh_cb)(apa102_t* sender, apa102_color_t* color, size_t len, void* context);

extern apa102_driver_t APA102;

static int apa102_init(int n);
static int apa102_update();
static int apa102_deinit();
void init_apa102_driver(apa102_driver_t *driver);

#define LEDSTRIP APA102