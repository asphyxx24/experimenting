#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "display.h"
#include "touch.h"
#include "ptt.h"
#include "sprite_engine.h"
#include "pig_sprites.h"

static const char *TAG = "main";

// Aufnahme-Indikator: schmaler Balken oben mittig. Der Finger verdeckt beim
// Halten die Displaymitte, oben bleibt er sichtbar.
#define REC_W 40
#define REC_H 6
#define REC_X ((DISPLAY_WIDTH - REC_W) / 2)
#define REC_Y 20

#define COLOR_REC   0xF800   // rot (RGB565 — bei falscher Farbe Byte-Order pruefen)
#define COLOR_CLEAR 0x0000

static uint16_t rec_buf[REC_W * REC_H];

static void draw_rec_indicator(uint16_t color)
{
    for (int i = 0; i < REC_W * REC_H; i++) {
        rec_buf[i] = color;
    }
    // esp_lcd_panel_draw_bitmap: x_end/y_end sind exklusiv.
    display_draw_bitmap(REC_X, REC_Y, REC_X + REC_W, REC_Y + REC_H, rec_buf);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Companion Watch starting...");

    mic_init();
    xTaskCreate(mic_task, "mic", 4096, NULL, 5, NULL);

    display_init();
    display_clear(COLOR_CLEAR);

    touch_init();
    ptt_init();
    sprite_engine_init();
    sprite_engine_load(pig_animations, PIG_ANIMATION_COUNT);

    ESP_LOGI(TAG, "Ready — tap switches animation, hold to talk");

    bool was_recording = false;

    while (1) {
        switch (ptt_update()) {
        case PTT_EVENT_TAP:
            sprite_engine_next_animation();
            break;

        case PTT_EVENT_START:
            // TODO: I2S-Aufnahme starten (Mikro ist verloetet, Pins noch offen)
            break;

        case PTT_EVENT_STOP:
            // TODO: Aufnahme beenden und an das Jarvis-Backend senden
            break;

        case PTT_EVENT_ABORT:
            // TODO: laufende Aufnahme verwerfen
            break;

        default:
            break;
        }

        sprite_engine_update();
        sprite_engine_render();

        bool recording = ptt_is_recording();
        if (recording) {
            draw_rec_indicator(COLOR_REC);
        } else if (was_recording) {
            // Genau einmal loeschen — der Sprite deckt den Balken nicht ab.
            draw_rec_indicator(COLOR_CLEAR);
        }
        was_recording = recording;

        vTaskDelay(pdMS_TO_TICKS(16));
    }
}
