#include "mic.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>

static const char *TAG = "mic";

#define MIC_SAMPLE_RATE  16000
#define MIC_BUF_SAMPLES  256

static i2s_chan_handle_t   s_rx_chan  = NULL;
static volatile int32_t   s_amplitude = 0;

static void mic_task(void *arg)
{
    int32_t buf[MIC_BUF_SAMPLES];
    size_t  bytes_read;

    while (1) {
        esp_err_t err = i2s_channel_read(s_rx_chan, buf, sizeof(buf), &bytes_read, pdMS_TO_TICKS(200));
        if (err != ESP_OK) continue;

        int samples = bytes_read / sizeof(int32_t);
        int32_t peak = 0;
        for (int i = 0; i < samples; i++) {
            // INMP441: 24-bit data left-justified in 32-bit frame → shift right 8 to get 24-bit value
            int32_t val = buf[i] >> 8;
            if (val < 0) val = -val;
            if (val > peak) peak = val;
        }
        s_amplitude = peak;
        ESP_LOGI(TAG, "peak=%ld", (long)peak);
    }
}

void mic_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &s_rx_chan));

    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(MIC_SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = MIC_BCLK_GPIO,
            .ws   = MIC_LRCLK_GPIO,
            .dout = I2S_GPIO_UNUSED,
            .din  = MIC_DIN_GPIO,
            .invert_flags = { false, false, false },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(s_rx_chan, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(s_rx_chan));

    xTaskCreate(mic_task, "mic_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "INMP441 ready  BCLK=%d  WS=%d  DIN=%d", MIC_BCLK_GPIO, MIC_LRCLK_GPIO, MIC_DIN_GPIO);
}

int32_t mic_get_amplitude(void)
{
    return s_amplitude;
}
