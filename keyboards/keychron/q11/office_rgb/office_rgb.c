#include QMK_KEYBOARD_H

#include "office_rgb.h"



#define OFFICE_HUE_STEP   8

#define OFFICE_BAND_SPEED (UINT8_MAX / 4)

static office_rgb_mode_t office_mode = OFFICE_RGB_OFF;

static uint8_t           left_hue    = 160;

static uint8_t           right_hue   = 28;



typedef enum {

    ZONE_MAIN = 0,

    ZONE_NAV,

    ZONE_FN_ROW,

    ZONE_MODIFIER,

} office_zone_t;



// 右手导航区（RCtrl 走修饰键配色，不在此列）

static const uint8_t nav_keys[][2] = {

    {7, 8},  // PgUp

    {8, 8},  // PgDn

    {9, 8},  // Home

    {10, 7}, // Up

    {11, 7}, // Left

    {11, 8}, // Down / Right

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



static office_zone_t office_zone_for_led(uint8_t led_idx) {

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



static uint8_t office_rgb_brightness(void) {

    return rgb_matrix_config.hsv.v;

}



static rgb_t office_zone_rgb(office_zone_t zone) {

    hsv_t hsv = {0, 0, office_rgb_brightness()};



    switch (zone) {

        case ZONE_FN_ROW:

            hsv.h = 200;

            hsv.s = 200;

            break;

        case ZONE_MODIFIER:

            hsv.h = 210;

            hsv.s = 80;

            hsv.v = (uint8_t)((uint16_t)90 * office_rgb_brightness() / UINT8_MAX);

            break;

        case ZONE_NAV:

            hsv.h = 160;

            hsv.s = 90;

            hsv.v = (uint8_t)((uint16_t)office_rgb_brightness() * 9 / 10);

            break;

        case ZONE_MAIN:

        default:

            hsv.h = 160;

            hsv.s = 120;

            break;

    }



    return hsv_to_rgb(hsv);

}



static void office_rgb_sync_hues(void) {

    rgb_matrix_sethsv_noeeprom(left_hue, right_hue, office_rgb_brightness());

}



static void office_rgb_apply_static_dual(void) {

    rgb_matrix_enable();

    rgb_matrix_set_flags_noeeprom(LED_FLAG_ALL);

    office_rgb_sync_hues();

    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_office_static_dual);

}



static void office_rgb_apply_static_zone(void) {

    rgb_matrix_enable();

    rgb_matrix_set_flags_noeeprom(LED_FLAG_ALL);

    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_office_static_zone);

}



static void office_rgb_apply_typing_ripple(void) {

    rgb_matrix_enable();

    rgb_matrix_set_flags_noeeprom(LED_FLAG_ALL);

    rgb_matrix_set_speed_noeeprom(64);

    rgb_matrix_sethsv_noeeprom(rgb_matrix_config.hsv.h, UINT8_MAX, office_rgb_brightness());

    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_office_typing_ripple);

}



static void office_rgb_apply_band_wave(void) {

    rgb_matrix_enable();

    rgb_matrix_set_flags_noeeprom(LED_FLAG_ALL);

    rgb_matrix_set_speed_noeeprom(OFFICE_BAND_SPEED);

    rgb_matrix_sethsv_noeeprom(rgb_matrix_config.hsv.h, UINT8_MAX, office_rgb_brightness());

    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_office_band_wave);

}



void office_rgb_apply_mode(office_rgb_mode_t mode) {

    office_mode = mode;



    switch (mode) {

        case OFFICE_RGB_OFF:

            rgb_matrix_disable();

            break;

        case OFFICE_RGB_MODE1:

            office_rgb_apply_static_dual();

            break;

        case OFFICE_RGB_MODE4:

            office_rgb_apply_static_zone();

            break;

        case OFFICE_RGB_MODE2:

            office_rgb_apply_typing_ripple();

            break;

        case OFFICE_RGB_MODE3:

            office_rgb_apply_band_wave();

            break;

        default:

            office_rgb_apply_mode(OFFICE_RGB_OFF);

            break;

    }

}



void office_rgb_cycle_mode(void) {

    office_rgb_mode_t next = (office_mode + 1) % OFFICE_RGB_MODE_COUNT;

    office_rgb_apply_mode(next);

}

void office_rgb_cycle_mode_reverse(void) {

    office_rgb_mode_t prev = (office_mode + OFFICE_RGB_MODE_COUNT - 1) % OFFICE_RGB_MODE_COUNT;

    office_rgb_apply_mode(prev);

}



office_rgb_mode_t office_rgb_get_mode(void) {

    return office_mode;

}



static void office_rgb_adjust_global_hue(bool decrease) {

    if (decrease) {

        rgb_matrix_decrease_hue_noeeprom();

    } else {

        rgb_matrix_increase_hue_noeeprom();

    }

}



static void office_rgb_adjust_left_hue(bool decrease) {

    if (decrease) {

        left_hue -= OFFICE_HUE_STEP;

    } else {

        left_hue += OFFICE_HUE_STEP;

    }

    office_rgb_sync_hues();

}



static void office_rgb_adjust_right_hue(bool decrease) {

    if (decrease) {

        right_hue -= OFFICE_HUE_STEP;

    } else {

        right_hue += OFFICE_HUE_STEP;

    }

    office_rgb_sync_hues();

}



bool office_rgb_process_enc_key(uint16_t keycode, keyrecord_t *record, bool enc_l_held) {

    if (!enc_l_held || office_mode == OFFICE_RGB_OFF || office_mode == OFFICE_RGB_MODE4) {

        return false;

    }



    if (!record->event.pressed) {

        return true;

    }



    switch (office_mode) {

        case OFFICE_RGB_MODE1:

            switch (keycode) {

                case KC_LEFT:

                    office_rgb_adjust_left_hue(true);

                    return true;

                case KC_RGHT:

                    office_rgb_adjust_left_hue(false);

                    return true;

                case KC_UP:

                    office_rgb_adjust_right_hue(false);

                    return true;

                case KC_DOWN:

                    office_rgb_adjust_right_hue(true);

                    return true;

                default:

                    return false;

            }



        case OFFICE_RGB_MODE2:

        case OFFICE_RGB_MODE3:

            switch (keycode) {

                case KC_LEFT:

                    office_rgb_adjust_global_hue(true);

                    return true;

                case KC_RGHT:

                    office_rgb_adjust_global_hue(false);

                    return true;

                default:

                    return false;

            }



        default:

            return false;

    }

}



void office_rgb_render_static(uint8_t led_min, uint8_t led_max) {

    const bool dual_mode = (rgb_matrix_config.mode == RGB_MATRIX_CUSTOM_office_static_dual);

    const bool zone_mode = (rgb_matrix_config.mode == RGB_MATRIX_CUSTOM_office_static_zone);



    if (!dual_mode && !zone_mode) {

        return;

    }



    const uint8_t left_hue_sync  = rgb_matrix_config.hsv.h;

    const uint8_t right_hue_sync = rgb_matrix_config.hsv.s;

    const uint8_t brightness     = office_rgb_brightness();



    for (uint8_t i = led_min; i < led_max; i++) {

        if (!HAS_ANY_FLAGS(g_led_config.flags[i], rgb_matrix_get_flags())) {

            continue;

        }

        rgb_t rgb;



        if (dual_mode) {

            hsv_t hsv = {g_led_config.point[i].x < OFFICE_SPLIT_X ? left_hue_sync : right_hue_sync, UINT8_MAX, brightness};

            rgb       = hsv_to_rgb(hsv);

        } else {

            rgb = office_zone_rgb(office_zone_for_led(i));

        }



        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);

    }

}



void keyboard_post_init_office_rgb(void) {

    office_rgb_apply_mode(OFFICE_RGB_OFF);

}


