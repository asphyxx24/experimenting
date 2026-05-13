#include "touch.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "touch";

#define PIN_SDA      6
#define PIN_SCL      7
#define PIN_RST      13
#define CST816S_ADDR 0x15

static i2c_master_dev_handle_t dev = NULL;
static bool prev_touched = false;

void touch_init(void)
{
    ESP_LOGI(TAG, "Initializing CST816S touch");

    gpio_config_t rst_cfg = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_RST,
    };
    gpio_config(&rst_cfg);
    gpio_set_level(PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(50));

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PIN_SDA,
        .scl_io_num = PIN_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = CST816S_ADDR,
        .scl_speed_hz = 400000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &dev));

    ESP_LOGI(TAG, "Touch ready");
}

bool touch_tapped(void)
{
    uint8_t reg = 0x02;
    uint8_t fingers = 0;

    esp_err_t ret = i2c_master_transmit_receive(dev, &reg, 1, &fingers, 1, pdMS_TO_TICKS(50));
    if (ret != ESP_OK) return false;

    bool touched = fingers > 0;
    bool tapped = touched && !prev_touched;
    prev_touched = touched;
    return tapped;
}
