#include QMK_KEYBOARD_H

#include "q11_rgb.h"
#include "q11_rgb_profile.h"

#if defined(RGB_MATRIX_ENABLE) && defined(RGB_MATRIX_SPLIT)
#    include "transport.h"
#endif

#define Q11_HUE_STEP   8
#define Q11_SAT_STEP   8
#define Q11_SAT_LEVELS 5
#define Q11_BAND_SPEED (UINT8_MAX / 4)

// 0 = 近白，255 = 最浓；中间档在同色相下由浅到深
static const uint8_t q11_sat_table[Q11_SAT_LEVELS] = {0, 48, 112, 180, 255};

static const q11_rgb_mode_t q11_rgb_cycle_modes[] = {Q11_RGB_CYCLE_LIST};
#define Q11_RGB_CYCLE_COUNT (sizeof(q11_rgb_cycle_modes) / sizeof(q11_rgb_cycle_modes[0]))

static q11_rgb_mode_t current_mode = Q11_RGB_OFF;

typedef struct {
    uint8_t hue;
    uint8_t left_sat_level;
    uint8_t right_sat_level;
} q11_solid_color_t;

typedef struct {
    uint8_t hue;
    uint8_t sat;
} q11_mono_color_t;

static q11_solid_color_t solid_color = {160, Q11_SAT_LEVELS - 1, Q11_SAT_LEVELS - 1};
static q11_mono_color_t ripple_color = {160, UINT8_MAX};
static q11_mono_color_t wave_color   = {160, UINT8_MAX};

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

static uint8_t sat_from_level(uint8_t level) {
    if (level >= Q11_SAT_LEVELS) {
        level = Q11_SAT_LEVELS - 1;
    }
    return q11_sat_table[level];
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

static uint8_t q11_solid_sat_for_this_half(void) {
    const uint8_t packed = rgb_matrix_config.speed;
#if defined(RGB_MATRIX_SPLIT)
    const uint8_t level = is_keyboard_left() ? (packed >> 4) : (packed & 0x0F);
#else
    const uint8_t level = packed >> 4;
#endif
    return sat_from_level(level);
}

static void apply_solid_colors(void) {
    rgb_matrix_config.hsv.h = solid_color.hue;
    rgb_matrix_config.hsv.s = sat_from_level(solid_color.left_sat_level);
    rgb_matrix_config.speed = ((solid_color.left_sat_level & 0x0F) << 4) | (solid_color.right_sat_level & 0x0F);
    q11_rgb_push_config();
}

static void apply_ripple_colors(void) {
    rgb_matrix_sethsv_noeeprom(ripple_color.hue, ripple_color.sat, q11_rgb_brightness());
}

static void apply_wave_colors(void) {
    rgb_matrix_sethsv_noeeprom(wave_color.hue, wave_color.sat, q11_rgb_brightness());
}

static void apply_static_dual(void) {
    rgb_matrix_enable();
    rgb_matrix_set_flags_noeeprom(LED_FLAG_ALL);
    apply_solid_colors();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_q11_static_dual);
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
    apply_ripple_colors();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_q11_typing_ripple);
}

static void apply_band_wave(void) {
    rgb_matrix_enable();
    rgb_matrix_set_flags_noeeprom(LED_FLAG_ALL);
    rgb_matrix_set_speed_noeeprom(Q11_BAND_SPEED);
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
            apply_static_dual();
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

static void adjust_hue(uint8_t *hue, bool decrease) {
    if (decrease) {
        *hue -= Q11_HUE_STEP;
    } else {
        *hue += Q11_HUE_STEP;
    }
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
                case KC_DOWN:
                    adjust_hue(&solid_color.hue, true);
                    apply_solid_colors();
                    return true;
                case KC_RGHT:
                case KC_UP:
                    adjust_hue(&solid_color.hue, false);
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
                    adjust_hue(&ripple_color.hue, true);
                    apply_ripple_colors();
                    return true;
                case KC_RGHT:
                    adjust_hue(&ripple_color.hue, false);
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
                    adjust_hue(&wave_color.hue, true);
                    apply_wave_colors();
                    return true;
                case KC_RGHT:
                    adjust_hue(&wave_color.hue, false);
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
    const bool dual_mode = (rgb_matrix_config.mode == RGB_MATRIX_CUSTOM_q11_static_dual);
    const bool zone_mode = (rgb_matrix_config.mode == RGB_MATRIX_CUSTOM_q11_static_zone);

    if (!dual_mode && !zone_mode) {
        return;
    }

    const uint8_t brightness = q11_rgb_brightness();

    for (uint8_t i = led_min; i < led_max; i++) {
        if (!HAS_ANY_FLAGS(g_led_config.flags[i], rgb_matrix_get_flags())) {
            continue;
        }

        rgb_t rgb;

        if (dual_mode) {
            hsv_t hsv = {
                rgb_matrix_config.hsv.h,
                q11_solid_sat_for_this_half(),
                brightness,
            };
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
