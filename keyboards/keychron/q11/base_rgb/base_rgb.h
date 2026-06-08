#pragma once

#include QMK_KEYBOARD_H

typedef enum {
    BASE_RGB_OFF = 0,
    BASE_RGB_MODE1, // 纯色常亮
    BASE_RGB_MODE2, // 纯色涟漪（打字触发）
    BASE_RGB_MODE3, // 纯色亮度带左→右滚动
    BASE_RGB_MODE_COUNT,
} base_rgb_mode_t;

void     base_rgb_apply_mode(base_rgb_mode_t mode);
void     base_rgb_cycle_mode(void);
base_rgb_mode_t base_rgb_get_mode(void);
bool     base_rgb_process_hue_key(uint16_t keycode, keyrecord_t *record, bool enc_l_held);
void     keyboard_post_init_base_rgb(void);
