#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "display.h"
#include "touch.h"
#include "sprite_engine.h"
#include "pig_sprites.h"

static const char *TAG = "main";

#define MIC_WS   GPIO_NUM_15
#define MIC_SCK  GPIO_NUM_16
#define MIC_SD   GPIO_NUM_17

static i2s_chan_handle_t mic_rx;

static void mic_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    i2s_new_channel(&chan_cfg, NULL, &mic_rx);

    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = MIC_SCK,
            .ws   = MIC_WS,
            .dout = I2S_GPIO_UNUSED,
            .din  = MIC_SD,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    i2s_channel_init_std_mode(mic_rx, &std_cfg);
    i2s_channel_enable(mic_rx);
}

static void mic_task(void *arg)
{
    int32_t raw[256];
    size_t bytes_read;

    // Countdown damit Python-Script bereit sein kann
    for (int i = 5; i > 0; i--) {
        printf("COUNTDOWN:%d\n", i);
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Phase 1: Aufnehmen (10 Sekunden)
    const int TOTAL = 16000 * 10;
    int16_t *samples = malloc(TOTAL * sizeof(int16_t));
    if (!samples) { ESP_LOGE("MIC", "malloc fehlgeschlagen"); vTaskDelete(NULL); return; }

    int pos = 0;
    printf("\nAUFNAHME — jetzt sprechen!\n");
    fflush(stdout);

    while (pos < TOTAL) {
        i2s_channel_read(mic_rx, raw, sizeof(raw), &bytes_read, pdMS_TO_TICKS(100));
        int n = bytes_read / sizeof(int32_t);
        for (int i = 0; i < n && pos < TOTAL; i++) {
            samples[pos++] = (int16_t)(raw[i] >> 16);
        }
    }

    // DC-Offset entfernen
    int64_t dc_sum = 0;
    for (int i = 0; i < TOTAL; i++) dc_sum += samples[i];
    int16_t dc = (int16_t)(dc_sum / TOTAL);
    for (int i = 0; i < TOTAL; i++) samples[i] -= dc;

    // Phase 2: Daten ausgeben
    printf("\nAUDIO_START\n");
    for (int i = 0; i < TOTAL; i++) printf("%d\n", samples[i]);
    printf("AUDIO_END\n");
    fflush(stdout);
    free(samples);
    printf("Fertig! Laeuft jetzt im Live-Modus.\n");

    // Phase 3: Live-Pegelmeter mit DC-Offset-Entfernung
    const int BAR = 40;
    const int32_t MAX_VAL = 8000;
    char bar[BAR + 1];

    while (1) {
        i2s_channel_read(mic_rx, raw, sizeof(raw), &bytes_read, pdMS_TO_TICKS(50));
        int n = bytes_read / sizeof(int32_t);

        int64_t sum = 0;
        for (int i = 0; i < n; i++) sum += (int16_t)(raw[i] >> 16);
        int32_t mean = (int32_t)(sum / n);

        int32_t peak = 0;
        for (int i = 0; i < n; i++) {
            int32_t s = (int16_t)(raw[i] >> 16) - mean;
            if (s < 0) s = -s;
            if (s > peak) peak = s;
        }
        int filled = (int)((long long)peak * BAR / MAX_VAL);
        if (filled > BAR) filled = BAR;
        for (int i = 0; i < BAR; i++) bar[i] = i < filled ? '#' : '.';
        bar[BAR] = '\0';
        printf("\r[%s] %ld      ", bar, (long)peak);
        fflush(stdout);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Companion Watch starting...");

    mic_init();
    xTaskCreate(mic_task, "mic", 4096, NULL, 5, NULL);

    display_init();
    display_clear(0x0000);

    touch_init();
    sprite_engine_init();
    sprite_engine_load(pig_animations, PIG_ANIMATION_COUNT);

    ESP_LOGI(TAG, "Ready — tap to switch animations (idle/walk/sleep)");

    while (1) {
        if (touch_tapped()) {
            sprite_engine_next_animation();
        }
        sprite_engine_update();
        sprite_engine_render();
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}
