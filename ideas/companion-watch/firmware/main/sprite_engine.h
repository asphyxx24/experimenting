#pragma once
#include <stdint.h>
#include <stdbool.h>

#define SPRITE_SIZE   32
#define SPRITE_SCALE  4
#define SPRITE_SCALED (SPRITE_SIZE * SPRITE_SCALE)

typedef struct {
    const uint16_t *frames;
    int frame_count;
    int frame_delay_ms;
    const char *name;
} sprite_animation_t;

void sprite_engine_init(void);
void sprite_engine_load(const sprite_animation_t *animations, int count);
void sprite_engine_next_animation(void);
void sprite_engine_update(void);
void sprite_engine_render(void);
