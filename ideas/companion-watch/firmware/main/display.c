#include "display.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_gc9a01.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

static const char *TAG = "display";

#define LCD_HOST    SPI2_HOST
#define PIN_MOSI    11
#define PIN_SCLK    10
#define PIN_CS      9
#define PIN_DC      8
#define PIN_BL      2
#define PIN_RST     14
#define LCD_FREQ_HZ (80 * 1000 * 1000)

static esp_lcd_panel_handle_t panel = NULL;

void display_init(void)
{
    ESP_LOGI(TAG, "Initializing GC9A01 display");

    gpio_config_t bl_cfg = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_BL,
    };
    gpio_config(&bl_cfg);
    gpio_set_level(PIN_BL, 1);

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * 2,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_io_spi_config_t io_cfg = {
        .dc_gpio_num = PIN_DC,
        .cs_gpio_num = PIN_CS,
        .pclk_hz = LCD_FREQ_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_cfg, &io));

    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io, &panel_cfg, &panel));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));

    ESP_LOGI(TAG, "Display ready");
}

void display_clear(uint16_t color)
{
    const int chunk = 10;
    size_t buf_size = DISPLAY_WIDTH * chunk * sizeof(uint16_t);
    uint16_t *buf = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);
    if (!buf) return;

    for (int i = 0; i < DISPLAY_WIDTH * chunk; i++) {
        buf[i] = color;
    }
    for (int y = 0; y < DISPLAY_HEIGHT; y += chunk) {
        int end_y = y + chunk;
        if (end_y > DISPLAY_HEIGHT) end_y = DISPLAY_HEIGHT;
        esp_lcd_panel_draw_bitmap(panel, 0, y, DISPLAY_WIDTH, end_y, buf);
    }
    free(buf);
}

void display_draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *data)
{
    esp_lcd_panel_draw_bitmap(panel, x_start, y_start, x_end, y_end, data);
}
