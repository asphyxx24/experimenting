#pragma once
#include <stdint.h>

#define MIC_BCLK_GPIO   4
#define MIC_LRCLK_GPIO  5
#define MIC_DIN_GPIO    3

void    mic_init(void);
int32_t mic_get_amplitude(void);
