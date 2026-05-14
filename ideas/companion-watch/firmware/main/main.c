#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "display.h"
#include "touch.h"
#include "sprite_engine.h"
#include "pig_sprites.h"
#include "mic.h"

static const char *TAG = "main";

// VU bar geometry (bottom strip, horizontally centered)
#define BAR_X       20
#define BAR_Y       210
#define BAR_W_MAX   200
#define BAR_H       16
#define MIC_PEAK_MAX 800000   // ~24-bit peak for typical voice at close range

static void draw_vu_bar(int32_t amplitude)
{
    int filled = (int)((int64_t)amplitude * BAR_W_MAX / MIC_PEAK_MAX);
    if (filled > BAR_W_MAX) filled = BAR_W_MAX;
    if (filled < 0)         filled = 0;

    // green filled portion, black remainder
    if (filled > 0)
        display_fill_rect(BAR_X, BAR_Y, filled, BAR_H, 0x07E0);          // green
    if (filled < BAR_W_MAX)
        display_fill_rect(BAR_X + filled, BAR_Y, BAR_W_MAX - filled, BAR_H, 0x0000); // black
}

void app_main(void)
{
    ESP_LOGI(TAG, "Companion Watch starting...");

    display_init();
    display_clear(0x0000);

    touch_init();
    sprite_engine_init();
    sprite_engine_load(pig_animations, PIG_ANIMATION_COUNT);

    mic_init();

    ESP_LOGI(TAG, "Ready — tap to switch animations, mic VU bar at bottom");

    while (1) {
        if (touch_tapped()) {
            sprite_engine_next_animation();
        }
        sprite_engine_update();
        sprite_engine_render();
        draw_vu_bar(mic_get_amplitude());
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}
