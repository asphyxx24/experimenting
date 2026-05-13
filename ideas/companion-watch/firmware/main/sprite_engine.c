#include "sprite_engine.h"
#include "display.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "sprite";

#define OFFSET ((DISPLAY_WIDTH - SPRITE_SCALED) / 2)

static const sprite_animation_t *anims = NULL;
static int num_anims = 0;
static int cur_anim = 0;
static int cur_frame = 0;
static int64_t last_frame_us = 0;
static uint16_t *render_buf = NULL;
static bool dirty = true;

/* ---- helpers ---- */

static uint16_t rgb565_be(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t c = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
    return (c >> 8) | (c << 8);
}

/* ---- placeholder sprites (replaced once real PNGs arrive) ---- */

#define PH_FRAMES 4
#define PH_ANIMS  3
#define PH_PX     (SPRITE_SIZE * SPRITE_SIZE)

static uint16_t ph_data[PH_ANIMS * PH_FRAMES * PH_PX];
static sprite_animation_t ph_anims[PH_ANIMS];

static void placeholder_init(void)
{
    uint16_t pink[] = {
        rgb565_be(255, 150, 150), rgb565_be(240, 130, 130),
        rgb565_be(220, 110, 110), rgb565_be(240, 130, 130),
    };
    for (int f = 0; f < PH_FRAMES; f++) {
        uint16_t *p = &ph_data[f * PH_PX];
        for (int i = 0; i < PH_PX; i++) p[i] = pink[f];
        p[10 * SPRITE_SIZE + 12] = 0x0000;
        p[10 * SPRITE_SIZE + 19] = 0x0000;
        p[16 * SPRITE_SIZE + 15] = rgb565_be(200, 100, 100);
        p[16 * SPRITE_SIZE + 16] = rgb565_be(200, 100, 100);
    }
    ph_anims[0] = (sprite_animation_t){
        .frames = &ph_data[0], .frame_count = PH_FRAMES,
        .frame_delay_ms = 300, .name = "idle",
    };

    for (int f = 0; f < PH_FRAMES; f++) {
        uint16_t *p = &ph_data[(PH_FRAMES + f) * PH_PX];
        for (int y = 0; y < SPRITE_SIZE; y++)
            for (int x = 0; x < SPRITE_SIZE; x++)
                p[y * SPRITE_SIZE + x] = ((x + f * 8) % SPRITE_SIZE < 16)
                    ? rgb565_be(100, 220, 100)
                    : rgb565_be(50, 150, 50);
    }
    ph_anims[1] = (sprite_animation_t){
        .frames = &ph_data[PH_FRAMES * PH_PX], .frame_count = PH_FRAMES,
        .frame_delay_ms = 200, .name = "walk",
    };

    uint16_t blue[] = {
        rgb565_be(80, 80, 200), rgb565_be(60, 60, 170),
        rgb565_be(40, 40, 140), rgb565_be(60, 60, 170),
    };
    for (int f = 0; f < PH_FRAMES; f++) {
        uint16_t *p = &ph_data[(PH_FRAMES * 2 + f) * PH_PX];
        for (int i = 0; i < PH_PX; i++) p[i] = blue[f];
    }
    ph_anims[2] = (sprite_animation_t){
        .frames = &ph_data[PH_FRAMES * 2 * PH_PX], .frame_count = PH_FRAMES,
        .frame_delay_ms = 500, .name = "sleep",
    };
}

/* ---- engine ---- */

void sprite_engine_init(void)
{
    render_buf = heap_caps_malloc(
        SPRITE_SCALED * SPRITE_SCALED * sizeof(uint16_t), MALLOC_CAP_DMA);
    assert(render_buf);

    placeholder_init();
    sprite_engine_load(ph_anims, PH_ANIMS);

    ESP_LOGI(TAG, "Sprite engine ready (placeholder)");
}

void sprite_engine_load(const sprite_animation_t *animations, int count)
{
    anims = animations;
    num_anims = count;
    cur_anim = 0;
    cur_frame = 0;
    last_frame_us = esp_timer_get_time();
    dirty = true;
}

void sprite_engine_next_animation(void)
{
    cur_anim = (cur_anim + 1) % num_anims;
    cur_frame = 0;
    last_frame_us = esp_timer_get_time();
    dirty = true;
    ESP_LOGI(TAG, "-> %s", anims[cur_anim].name);
}

void sprite_engine_update(void)
{
    int64_t now = esp_timer_get_time();
    if (now - last_frame_us >= anims[cur_anim].frame_delay_ms * 1000) {
        cur_frame = (cur_frame + 1) % anims[cur_anim].frame_count;
        last_frame_us = now;
        dirty = true;
    }
}

void sprite_engine_render(void)
{
    if (!dirty) return;
    dirty = false;

    const uint16_t *frame =
        &anims[cur_anim].frames[cur_frame * SPRITE_SIZE * SPRITE_SIZE];

    for (int y = 0; y < SPRITE_SCALED; y++) {
        int sy = y / SPRITE_SCALE;
        const uint16_t *src = &frame[sy * SPRITE_SIZE];
        uint16_t *dst = &render_buf[y * SPRITE_SCALED];
        for (int x = 0; x < SPRITE_SCALED; x++) {
            dst[x] = src[x / SPRITE_SCALE];
        }
    }

    display_draw_bitmap(OFFSET, OFFSET,
                        OFFSET + SPRITE_SCALED, OFFSET + SPRITE_SCALED,
                        render_buf);
}
