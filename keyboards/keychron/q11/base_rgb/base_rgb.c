#include "base_rgb.h"

static base_rgb_mode_t base_rgb_mode = BASE_RGB_OFF;

static void base_rgb_apply_solid(void) {
    rgb_matrix_enable();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    rgb_matrix_sethsv_noeeprom(rgb_matrix_config.hsv.h, UINT8_MAX, BASE_RGB_FIXED_VAL);
}

static void base_rgb_apply_reactive(uint8_t mode) {
    rgb_matrix_enable();
    rgb_matrix_mode_noeeprom(mode);
    rgb_matrix_sethsv_noeeprom(rgb_matrix_config.hsv.h, UINT8_MAX, BASE_RGB_FIXED_VAL);
}

void base_rgb_apply_mode(base_rgb_mode_t mode) {
    base_rgb_mode = mode;

    switch (mode) {
        case BASE_RGB_OFF:
            rgb_matrix_disable();
            break;
        case BASE_RGB_MODE1:
            base_rgb_apply_solid();
            break;
        case BASE_RGB_MODE2:
            base_rgb_apply_reactive(RGB_MATRIX_SOLID_SPLASH);
            break;
        case BASE_RGB_MODE3:
            base_rgb_apply_reactive(RGB_MATRIX_BAND_VAL);
            break;
        default:
            base_rgb_apply_mode(BASE_RGB_OFF);
            break;
    }
}

void base_rgb_cycle_mode(void) {
    base_rgb_mode_t next = (base_rgb_mode + 1) % BASE_RGB_MODE_COUNT;
    base_rgb_apply_mode(next);
}

base_rgb_mode_t base_rgb_get_mode(void) {
    return base_rgb_mode;
}

bool base_rgb_process_hue_key(uint16_t keycode, keyrecord_t *record, bool enc_l_held) {
    if (!enc_l_held || base_rgb_mode == BASE_RGB_OFF) {
        return false;
    }

    if (!record->event.pressed) {
        return true;
    }

    switch (keycode) {
        case KC_LEFT:
            tap_code16(RM_HUED);
            return true;
        case KC_RGHT:
            tap_code16(RM_HUEU);
            return true;
        default:
            return false;
    }
}

void keyboard_post_init_base_rgb(void) {
    base_rgb_apply_mode(BASE_RGB_OFF);
}
