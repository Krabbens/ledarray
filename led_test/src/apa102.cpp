#include <apa102.h>

#define TAG "APA102"

// Function prototypes (declare them before using)
spi_bus_config_t init_spi_bus_config();
spi_device_interface_config_t init_spi_device_interface_config();

// Initialization function for apa102_driver_t
void init_apa102_driver(apa102_driver_t *driver)
{
    driver->phase = 0;
    driver->device = 0;
    driver->spi_host = CONFIG_LMTZ_APA102_SPI_HOST;
    driver->dma_channel = CONFIG_LMTZ_APA102_DMA_CHANNEL;
    driver->bus_config = init_spi_bus_config();
    driver->dev_config = init_spi_device_interface_config();

    driver->init = apa102_init;
    driver->update = apa102_update;
    driver->refresh = NULL;
    driver->deinit = apa102_deinit;
}

// Implementations
spi_bus_config_t init_spi_bus_config()
{
    spi_bus_config_t config = {};
    config.miso_io_num = -1;
    config.mosi_io_num = CONFIG_LMTZ_APA102_PIN_MOSI;
    config.sclk_io_num = CONFIG_LMTZ_APA102_PIN_SCLK;
    config.quadwp_io_num = -1;
    config.quadhd_io_num = -1;
    // Include other fields if necessary
    return config;
}

spi_device_interface_config_t init_spi_device_interface_config()
{
    spi_device_interface_config_t config = {};
    config.clock_speed_hz = CONFIG_LMTZ_APA102_CLOCK_SPEED;
    config.mode = 3;
    config.spics_io_num = -1;
    config.queue_size = 1;
    // Include other fields if necessary
    return config;
}

inline int apa102_max_transfer(int nleds)
{
    return (8 * ((2 + nleds) * sizeof(apa102_color_t)));
}

static int apa102_init(int n)
{
    APA102.count = (n <= 0) ? CONFIG_LMTZ_APA102_NUM_LEDS : n;
    APA102.transaction.length = apa102_max_transfer(APA102.count);
    APA102.bus_config.max_transfer_sz = APA102.transaction.length;
    size_t size = (APA102.count + 2) * sizeof(apa102_color_t);
    APA102.txbuffer = static_cast<apa102_color_t *>(heap_caps_malloc(size, MALLOC_CAP_DMA | MALLOC_CAP_32BIT));
    memset(APA102.txbuffer, 0, size);

    for (int i = 0; i < APA102.count; i++)
        APA102.txbuffer[1 + i] = 0xE0000000;
    esp_err_t ret = spi_bus_initialize(APA102.spi_host, &APA102.bus_config, APA102.dma_channel);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to init bus (%s)", esp_err_to_name(ret));
        return ret;
    }

    // Make sure this call succeeds
    int freq = 0;
    
    ret = spi_bus_add_device(APA102.spi_host, &APA102.dev_config, &APA102.device);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add device to SPI bus (%s)", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

static int apa102_update()
{
    if (!APA102.transaction.tx_buffer)
    {
        APA102.transaction.tx_buffer = APA102.txbuffer;
    }

    spi_device_queue_trans(APA102.device, &APA102.transaction, portMAX_DELAY);
    if (APA102.refresh)
        APA102.refresh();
    spi_transaction_t *t;
    spi_device_get_trans_result(APA102.device, &t, portMAX_DELAY);
    APA102.phase += 1;

    return ESP_OK;
}

static int apa102_deinit()
{
    free(APA102.txbuffer);
    return ESP_OK;
}