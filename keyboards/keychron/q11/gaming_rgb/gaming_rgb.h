#pragma once



#include <stdbool.h>

#include <stdint.h>



typedef struct keyrecord_t keyrecord_t;



// 模式称谓（循环顺序）：关闭 → 纯色

typedef enum {

    GAMING_RGB_OFF = 0, // 关闭

    GAMING_RGB_SOLID,   // 纯色

    GAMING_RGB_MODE_COUNT,

} gaming_rgb_mode_t;



void              gaming_rgb_apply_mode(gaming_rgb_mode_t mode);

void              gaming_rgb_cycle_mode(void);

void              gaming_rgb_cycle_mode_reverse(void);

gaming_rgb_mode_t gaming_rgb_get_mode(void);

bool              gaming_rgb_process_enc_key(uint16_t keycode, keyrecord_t *record, bool enc_l_held);

void              keyboard_post_init_gaming_rgb(void);


