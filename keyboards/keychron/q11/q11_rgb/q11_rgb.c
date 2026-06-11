#include QMK_KEYBOARD_H

#include "q11_rgb.h"
#include "q11_rgb_profile.h"

#if defined(RGB_MATRIX_ENABLE) && defined(RGB_MATRIX_SPLIT)
#    include "transport.h"
#endif

#define Q11_SAT_STEP   8
#define Q11_SAT_LEVELS 8
#define Q11_BAND_SPEED (UINT8_MAX / 4)

// keyboard.json rgb_matrix.split_count: 42 + 47
#define Q11_RGB_LEFT_LED_COUNT 42

// 色环：8 标准色 + 8 过渡（相邻标准色中点）；←/→ / ↑/↓ 在表中逐步切换，避免步进 8 导致相邻档肉眼难分
static const uint8_t q11_hue_table[] = {
    0,   // 红
    10,  // 红→橙
    21,  // 橙
    32,  // 橙→黄
    43,  // 黄
    64,  // 黄→绿
    85,  // 绿
    106, // 绿→青
    128, // 青
    149, // 青→蓝
    170, // 蓝
    180, // 蓝→紫
    191, // 紫
    202, // 紫→品红
    213, // 品红
    234, // 品红→红
};
#define Q11_HUE_COUNT (sizeof(q11_hue_table) / sizeof(q11_hue_table[0]))

// 饱和度 8 档：去掉近白低段（原 0–175），只在可见色范围内细分；255↔251 步进 4
static const uint8_t q11_sat_table[Q11_SAT_LEVELS] = {205, 215, 225, 233, 240, 246, 251, 255};

static const q11_rgb_mode_t q11_rgb_cycle_modes[] = {Q11_RGB_CYCLE_LIST};
#define Q11_RGB_CYCLE_COUNT (sizeof(q11_rgb_cycle_modes) / sizeof(q11_rgb_cycle_modes[0]))

static q11_rgb_mode_t current_mode = Q11_RGB_OFF;

typedef struct {
    uint8_t left_hue;
    uint8_t right_hue;
    uint8_t left_sat_level;
    uint8_t right_sat_level;
} q11_solid_color_t;

typedef struct {
    uint8_t hue;
    uint8_t sat;
} q11_mono_color_t;

static q11_solid_color_t solid_color = {128, 128, Q11_SAT_LEVELS - 1, Q11_SAT_LEVELS - 1};
static q11_mono_color_t ripple_color = {128, UINT8_MAX};
static q11_mono_color_t wave_color   = {128, UINT8_MAX};

typedef enum {
    ZONE_MAIN = 0,
    ZONE_NAV,
    ZONE_FN_ROW,
    ZONE_MODIFIER,
} q11_zone_t;

static const uint8_t nav_keys[][2] = {
    {7, 8},
    {8, 8},
    {9, 8},
    {10, 7},
    {11, 7},
    {11, 8},
};

static bool led_to_matrix(uint8_t led_idx, uint8_t *out_row, uint8_t *out_col) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (g_led_config.matrix_co[row][col] == led_idx) {
                *out_row = row;
                *out_col = col;
                return true;
            }
        }
    }
    return false;
}

static bool is_nav_key(uint8_t row, uint8_t col) {
    for (size_t i = 0; i < ARRAY_SIZE(nav_keys); i++) {
        if (nav_keys[i][0] == row && nav_keys[i][1] == col) {
            return true;
        }
    }
    return false;
}

static q11_zone_t zone_for_led(uint8_t led_idx) {
    if (g_led_config.point[led_idx].y <= 15) {
        return ZONE_FN_ROW;
    }

    if (HAS_FLAGS(g_led_config.flags[led_idx], LED_FLAG_MODIFIER)) {
        return ZONE_MODIFIER;
    }

    uint8_t row;
    uint8_t col;

    if (led_to_matrix(led_idx, &row, &col) && is_nav_key(row, col)) {
        return ZONE_NAV;
    }

    return ZONE_MAIN;
}

static uint8_t q11_rgb_brightness(void) {
    return rgb_matrix_config.hsv.v;
}

static rgb_t zone_rgb(q11_zone_t zone) {
    hsv_t hsv = {0, 0, q11_rgb_brightness()};

    switch (zone) {
        case ZONE_FN_ROW:
            hsv.h = 200;
            hsv.s = 200;
            break;
        case ZONE_MODIFIER:
            hsv.h = 210;
            hsv.s = 80;
            hsv.v = (uint8_t)((uint16_t)90 * q11_rgb_brightness() / UINT8_MAX);
            break;
        case ZONE_NAV:
            hsv.h = 160;
            hsv.s = 90;
            hsv.v = (uint8_t)((uint16_t)q11_rgb_brightness() * 9 / 10);
            break;
        case ZONE_MAIN:
        default:
            hsv.h = 160;
            hsv.s = 120;
            break;
    }

    return hsv_to_rgb(hsv);
}

// 黄/青满饱和时已有两路 RGB 顶格，再降 S 主要抬第三路 → 很快趋近白；单独抬高低端
static uint8_t sat_from_level_for_hue(uint8_t hue, uint8_t level) {
    if (level >= Q11_SAT_LEVELS) {
        level = Q11_SAT_LEVELS - 1;
    }

    if (hue >= 18 && hue <= 72) {
        static const uint8_t yellow_sat_table[Q11_SAT_LEVELS] = {228, 234, 238, 242, 246, 250, 253, 255};
        return yellow_sat_table[level];
    }

    if (hue >= 100 && hue <= 155) {
        static const uint8_t cyan_sat_table[Q11_SAT_LEVELS] = {222, 228, 233, 238, 243, 248, 252, 255};
        return cyan_sat_table[level];
    }

    return q11_sat_table[level];
}

static uint8_t q11_hue_distance(uint8_t a, uint8_t b) {
    uint8_t d = a > b ? a - b : b - a;
    return d <= 127 ? d : 255 - d;
}

static uint8_t q11_hue_index_for_value(uint8_t hue) {
    uint8_t best      = 0;
    uint8_t best_dist = 255;

    for (uint8_t i = 0; i < Q11_HUE_COUNT; i++) {
        uint8_t dist = q11_hue_distance(hue, q11_hue_table[i]);
        if (dist < best_dist) {
            best_dist = dist;
            best      = i;
        }
    }
    return best;
}

static void q11_hue_snap(uint8_t *hue) {
    *hue = q11_hue_table[q11_hue_index_for_value(*hue)];
}

static void cycle_hue(uint8_t *hue, bool decrease) {
    uint8_t idx = q11_hue_index_for_value(*hue);

    if (decrease) {
        idx = (idx + Q11_HUE_COUNT - 1) % Q11_HUE_COUNT;
    } else {
        idx = (idx + 1) % Q11_HUE_COUNT;
    }

    *hue = q11_hue_table[idx];
}

#if defined(RGB_MATRIX_ENABLE) && defined(RGB_MATRIX_SPLIT)
static void q11_rgb_push_config(void) {
    if (!is_keyboard_master()) {
        return;
    }

    rgb_matrix_sync_t sync;
    memcpy(&sync.rgb_matrix, &rgb_matrix_config, sizeof(rgb_config_t));
    sync.rgb_suspend_state = rgb_matrix_get_suspend_state();
    transport_execute_transaction(PUT_RGB_MATRIX, &sync, sizeof(sync), NULL, 0);
}
#else
static void q11_rgb_push_config(void) {}
#endif

// 纯色模式 split 同步：复用 rgb_matrix_config 字段（仅 Q11_RGB_SOLID 渲染时生效）
//   hsv.h   → 左半色相
//   hsv.s   → 右半色相（此处不是 HSV 饱和度）
//   speed   → 高 4 位左半 sat 档，低 4 位右半 sat 档

static bool q11_led_is_left_half(uint8_t led_idx) {
    return led_idx < Q11_RGB_LEFT_LED_COUNT;
}

static void q11_solid_hsv_for_led(uint8_t led_idx, hsv_t *hsv) {
    const uint8_t packed = rgb_matrix_config.speed;

    hsv->v = q11_rgb_brightness();
    if (q11_led_is_left_half(led_idx)) {
        hsv->h = rgb_matrix_config.hsv.h;
        hsv->s = sat_from_level_for_hue(hsv->h, packed >> 4);
    } else {
        hsv->h = rgb_matrix_config.hsv.s;
        hsv->s = sat_from_level_for_hue(hsv->h, packed & 0x0F);
    }
}

static void apply_solid_colors(void) {
    rgb_matrix_config.hsv.h = solid_color.left_hue;
    rgb_matrix_config.hsv.s = solid_color.right_hue;
    rgb_matrix_config.speed = ((solid_color.left_sat_level & 0x0F) << 4) | (solid_color.right_sat_level & 0x0F);
    q11_rgb_push_config();
}

static void apply_ripple_colors(void) {
    rgb_matrix_sethsv_noeeprom(ripple_color.hue, ripple_color.sat, q11_rgb_brightness());
}

static void apply_wave_colors(void) {
    rgb_matrix_sethsv_noeeprom(wave_color.hue, wave_color.sat, q11_rgb_brightness());
}

static void apply_solid(void) {
    rgb_matrix_enable();
    rgb_matrix_set_flags_noeeprom(LED_FLAG_ALL);
    q11_hue_snap(&solid_color.left_hue);
    q11_hue_snap(&solid_color.right_hue);
    apply_solid_colors();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_q11_static_solid);
}

static void apply_static_zone(void) {
    rgb_matrix_enable();
    rgb_matrix_set_flags_noeeprom(LED_FLAG_ALL);
    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_q11_static_zone);
}

static void apply_typing_ripple(void) {
    rgb_matrix_enable();
    rgb_matrix_set_flags_noeeprom(LED_FLAG_ALL);
    rgb_matrix_set_speed_noeeprom(64);
    q11_hue_snap(&ripple_color.hue);
    apply_ripple_colors();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_q11_typing_ripple);
}

static void apply_band_wave(void) {
    rgb_matrix_enable();
    rgb_matrix_set_flags_noeeprom(LED_FLAG_ALL);
    rgb_matrix_set_speed_noeeprom(Q11_BAND_SPEED);
    q11_hue_snap(&wave_color.hue);
    apply_wave_colors();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_q11_band_wave);
}

void q11_rgb_apply_mode(q11_rgb_mode_t mode) {
    current_mode = mode;

    switch (mode) {
        case Q11_RGB_OFF:
            rgb_matrix_disable();
            break;
        case Q11_RGB_SOLID:
            apply_solid();
            break;
        case Q11_RGB_ZONE:
            apply_static_zone();
            break;
        case Q11_RGB_RIPPLE:
            apply_typing_ripple();
            break;
        case Q11_RGB_WAVE:
            apply_band_wave();
            break;
        default:
            q11_rgb_apply_mode(Q11_RGB_OFF);
            break;
    }
}

static int cycle_index_for_mode(q11_rgb_mode_t mode) {
    for (size_t i = 0; i < Q11_RGB_CYCLE_COUNT; i++) {
        if (q11_rgb_cycle_modes[i] == mode) {
            return (int)i;
        }
    }
    return 0;
}

void q11_rgb_cycle_mode(void) {
    int idx = cycle_index_for_mode(current_mode);
    idx     = (idx + 1) % (int)Q11_RGB_CYCLE_COUNT;
    q11_rgb_apply_mode(q11_rgb_cycle_modes[idx]);
}

void q11_rgb_cycle_mode_reverse(void) {
    int idx = cycle_index_for_mode(current_mode);
    idx     = (idx + (int)Q11_RGB_CYCLE_COUNT - 1) % (int)Q11_RGB_CYCLE_COUNT;
    q11_rgb_apply_mode(q11_rgb_cycle_modes[idx]);
}

q11_rgb_mode_t q11_rgb_get_mode(void) {
    return current_mode;
}

static void adjust_sat_level(uint8_t *level, bool decrease) {
    if (decrease) {
        if (*level > 0) {
            (*level)--;
        }
    } else if (*level < Q11_SAT_LEVELS - 1) {
        (*level)++;
    }
}

static void adjust_sat(uint8_t *sat, bool decrease) {
    if (decrease) {
        if (*sat > Q11_SAT_STEP) {
            *sat -= Q11_SAT_STEP;
        } else {
            *sat = 0;
        }
    } else if (*sat < UINT8_MAX - Q11_SAT_STEP) {
        *sat += Q11_SAT_STEP;
    } else {
        *sat = UINT8_MAX;
    }
}

bool q11_rgb_process_enc_key(uint16_t keycode, keyrecord_t *record, bool enc_l_held) {
    if (!enc_l_held || current_mode == Q11_RGB_OFF || current_mode == Q11_RGB_ZONE) {
        return false;
    }

    if (!record->event.pressed) {
        return true;
    }

    switch (current_mode) {
        case Q11_RGB_SOLID:
            switch (keycode) {
                case KC_LEFT:
                    cycle_hue(&solid_color.left_hue, false);
                    apply_solid_colors();
                    return true;
                case KC_RGHT:
                    cycle_hue(&solid_color.left_hue, true);
                    apply_solid_colors();
                    return true;
                case KC_UP:
                    cycle_hue(&solid_color.right_hue, false);
                    apply_solid_colors();
                    return true;
                case KC_DOWN:
                    cycle_hue(&solid_color.right_hue, true);
                    apply_solid_colors();
                    return true;
                case KC_PGUP:
                    adjust_sat_level(&solid_color.left_sat_level, false);
                    apply_solid_colors();
                    return true;
                case KC_PGDN:
                    adjust_sat_level(&solid_color.left_sat_level, true);
                    apply_solid_colors();
                    return true;
                case KC_INS:
                    adjust_sat_level(&solid_color.right_sat_level, false);
                    apply_solid_colors();
                    return true;
                case KC_DEL:
                    adjust_sat_level(&solid_color.right_sat_level, true);
                    apply_solid_colors();
                    return true;
                default:
                    return false;
            }

        case Q11_RGB_RIPPLE:
            switch (keycode) {
                case KC_LEFT:
                    cycle_hue(&ripple_color.hue, true);
                    apply_ripple_colors();
                    return true;
                case KC_RGHT:
                    cycle_hue(&ripple_color.hue, false);
                    apply_ripple_colors();
                    return true;
                case KC_PGUP:
                    adjust_sat(&ripple_color.sat, false);
                    apply_ripple_colors();
                    return true;
                case KC_PGDN:
                    adjust_sat(&ripple_color.sat, true);
                    apply_ripple_colors();
                    return true;
                default:
                    return false;
            }

        case Q11_RGB_WAVE:
            switch (keycode) {
                case KC_LEFT:
                    cycle_hue(&wave_color.hue, true);
                    apply_wave_colors();
                    return true;
                case KC_RGHT:
                    cycle_hue(&wave_color.hue, false);
                    apply_wave_colors();
                    return true;
                case KC_PGUP:
                    adjust_sat(&wave_color.sat, false);
                    apply_wave_colors();
                    return true;
                case KC_PGDN:
                    adjust_sat(&wave_color.sat, true);
                    apply_wave_colors();
                    return true;
                default:
                    return false;
            }

        default:
            return false;
    }
}

void q11_rgb_render_static(uint8_t led_min, uint8_t led_max) {
    const bool solid_mode = (rgb_matrix_config.mode == RGB_MATRIX_CUSTOM_q11_static_solid);
    const bool zone_mode  = (rgb_matrix_config.mode == RGB_MATRIX_CUSTOM_q11_static_zone);

    if (!solid_mode && !zone_mode) {
        return;
    }

    for (uint8_t i = led_min; i < led_max; i++) {
        if (!HAS_ANY_FLAGS(g_led_config.flags[i], rgb_matrix_get_flags())) {
            continue;
        }

        rgb_t rgb;

        if (solid_mode) {
            hsv_t hsv;
            q11_solid_hsv_for_led(i, &hsv);
            rgb = hsv_to_rgb(hsv);
        } else {
            rgb = zone_rgb(zone_for_led(i));
        }

        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
}

void keyboard_post_init_q11_rgb(void) {
    q11_rgb_apply_mode(Q11_RGB_OFF);
}
