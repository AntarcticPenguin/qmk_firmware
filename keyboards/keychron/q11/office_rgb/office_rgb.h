#pragma once



#include <stdbool.h>

#include <stdint.h>



typedef struct keyrecord_t keyrecord_t;



// 模式称谓（循环顺序）：关闭 → 纯色 → 纯色涟漪 → 纯色波浪 → 办公彩

typedef enum {

    OFFICE_RGB_OFF = 0, // 关闭

    OFFICE_RGB_MODE1,   // 纯色（左右半区双色常亮）

    OFFICE_RGB_MODE2,   // 纯色涟漪（打字光晕反馈）

    OFFICE_RGB_MODE3,   // 纯色波浪

    OFFICE_RGB_MODE4,   // 办公彩

    OFFICE_RGB_MODE_COUNT,

} office_rgb_mode_t;



// 非 split 键盘回退用；Q11 纯色模式按 RGB_MATRIX_SPLIT LED 索引分左右半区
#define OFFICE_SPLIT_X 108



void              office_rgb_apply_mode(office_rgb_mode_t mode);

void              office_rgb_cycle_mode(void);

void              office_rgb_cycle_mode_reverse(void);

office_rgb_mode_t office_rgb_get_mode(void);

bool              office_rgb_process_enc_key(uint16_t keycode, keyrecord_t *record, bool enc_l_held);

void              office_rgb_render_static(uint8_t led_min, uint8_t led_max);

void              keyboard_post_init_office_rgb(void);


