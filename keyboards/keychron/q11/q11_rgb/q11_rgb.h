#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct keyrecord_t keyrecord_t;

// 模式 ID（循环列表由 q11_rgb_profile.h / keymap config 决定）
typedef enum {
    Q11_RGB_OFF = 0,
    Q11_RGB_SOLID,  // 纯色（←/→ 左半色相，↑/↓ 右半色相，PgUp/PgDn / Ins/Del 左右饱和度）
    Q11_RGB_RIPPLE, // 纯色涟漪
    Q11_RGB_WAVE,   // 纯色波浪
    Q11_RGB_ZONE,   // 分区固定配色
} q11_rgb_mode_t;

void           q11_rgb_apply_mode(q11_rgb_mode_t mode);
void           q11_rgb_cycle_mode(void);
void           q11_rgb_cycle_mode_reverse(void);
q11_rgb_mode_t q11_rgb_get_mode(void);
bool           q11_rgb_process_enc_key(uint16_t keycode, keyrecord_t *record, bool enc_l_held);
void           q11_rgb_render_static(uint8_t led_min, uint8_t led_max);
void           keyboard_post_init_q11_rgb(void);
