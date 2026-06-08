#include QMK_KEYBOARD_H

#include "gaming_rgb.h"



#define GAMING_HUE_STEP 8

#define GAMING_SAT_STEP 8



static gaming_rgb_mode_t gaming_mode = GAMING_RGB_OFF;



static uint8_t gaming_rgb_brightness(void) {

    return rgb_matrix_config.hsv.v;

}



static void gaming_rgb_apply_solid(void) {

    rgb_matrix_enable();

    rgb_matrix_set_flags_noeeprom(LED_FLAG_ALL);

    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);

    rgb_matrix_sethsv_noeeprom(rgb_matrix_config.hsv.h, rgb_matrix_config.hsv.s, gaming_rgb_brightness());

}



void gaming_rgb_apply_mode(gaming_rgb_mode_t mode) {

    gaming_mode = mode;



    switch (mode) {

        case GAMING_RGB_OFF:

            rgb_matrix_disable();

            break;

        case GAMING_RGB_SOLID:

            gaming_rgb_apply_solid();

            break;

        default:

            gaming_rgb_apply_mode(GAMING_RGB_OFF);

            break;

    }

}



void gaming_rgb_cycle_mode(void) {

    gaming_rgb_mode_t next = (gaming_mode + 1) % GAMING_RGB_MODE_COUNT;

    gaming_rgb_apply_mode(next);

}



void gaming_rgb_cycle_mode_reverse(void) {

    gaming_rgb_mode_t prev = (gaming_mode + GAMING_RGB_MODE_COUNT - 1) % GAMING_RGB_MODE_COUNT;

    gaming_rgb_apply_mode(prev);

}



gaming_rgb_mode_t gaming_rgb_get_mode(void) {

    return gaming_mode;

}



bool gaming_rgb_process_enc_key(uint16_t keycode, keyrecord_t *record, bool enc_l_held) {

    if (!enc_l_held || gaming_mode == GAMING_RGB_OFF) {

        return false;

    }



    if (!record->event.pressed) {

        return true;

    }



    if (gaming_mode != GAMING_RGB_SOLID) {

        return false;

    }



    switch (keycode) {

        case KC_LEFT:

            rgb_matrix_decrease_hue_noeeprom();

            return true;

        case KC_RGHT:

            rgb_matrix_increase_hue_noeeprom();

            return true;

        case KC_UP:

            rgb_matrix_increase_sat_noeeprom();

            return true;

        case KC_DOWN:

            rgb_matrix_decrease_sat_noeeprom();

            return true;

        default:

            return false;

    }

}



void keyboard_post_init_gaming_rgb(void) {

    gaming_rgb_apply_mode(GAMING_RGB_OFF);

}


