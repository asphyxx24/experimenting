#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "display.h"
#include "touch.h"
#include "sprite_engine.h"
#include "pig_sprites.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Companion Watch starting...");

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
