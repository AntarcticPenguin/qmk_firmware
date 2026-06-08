/* Copyright 2023 @ Keychron (https://www.keychron.com)

 *

 * This program is free software: you can redistribute it and/or modify

 * it under the terms of the GNU General Public License as published by

 * the Free Software Foundation, either version 2 of the License, or

 * (at your option) any later version.

 */

#include QMK_KEYBOARD_H

#include "../../../gaming_rgb/gaming_rgb.h"



enum layers {

    MAC_BASE,

    MAC_FN,

    WIN_BASE,

    WIN_FN,

};



#define KC_TASK LGUI(KC_TAB)

#define KC_FLXP LGUI(KC_E)



enum custom_keycodes {

    ENC_L = SAFE_RANGE,

};



#define ON_BASE_LAYER() (get_highest_layer(layer_state) == MAC_BASE || get_highest_layer(layer_state) == WIN_BASE)



#define GAMING_LEFT_SPC_ROW  5

#define GAMING_LEFT_SPC_COL  6

#define GAMING_RIGHT_SPC_ROW 11

#define GAMING_RIGHT_SPC_COL 1



typedef struct {

    bool     enc_l_held;

    bool     enc_l_chord;

    uint16_t enc_l_timer;

} enc_state_t;



static enc_state_t enc_state;



bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (keycode == ENC_L) {

        if (record->event.pressed) {

            enc_state.enc_l_held  = true;

            enc_state.enc_l_chord = false;

            enc_state.enc_l_timer = timer_read();

        } else {

            enc_state.enc_l_held  = false;

            enc_state.enc_l_chord = false;

        }

        return false;

    }



    if (!ON_BASE_LAYER()) {

        return true;

    }



    if (enc_state.enc_l_held) {

        switch (keycode) {

            case KC_LEFT:

            case KC_RGHT:

            case KC_UP:

            case KC_DOWN:

                gaming_rgb_process_enc_key(keycode, record, true);

                if (record->event.pressed) {

                    enc_state.enc_l_chord = true;

                }

                return false;

            case KC_SPC:

                if (record->event.pressed) {

                    enc_state.enc_l_chord = true;

                    if (record->event.key.row == GAMING_LEFT_SPC_ROW && record->event.key.col == GAMING_LEFT_SPC_COL) {

                        gaming_rgb_cycle_mode_reverse();

                    } else if (record->event.key.row == GAMING_RIGHT_SPC_ROW && record->event.key.col == GAMING_RIGHT_SPC_COL) {

                        gaming_rgb_cycle_mode();

                    }

                }

                return false;

            default:

                return false;

        }

    }



    return true;

}



void keyboard_post_init_user(void) {

    keyboard_post_init_gaming_rgb();

}



const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [MAC_BASE] = LAYOUT_91_ansi(

        ENC_L,    KC_ESC,   KC_BRID,  KC_BRIU,  KC_MCTL,  KC_LPAD,  _______,   _______,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,    KC_VOLU,  KC_INS,   KC_DEL,   KC_MUTE,

        _______,  KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,      KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,    KC_EQL,   KC_BSPC,            KC_PGUP,

        _______,  KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,      KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,    KC_RBRC,  KC_BSLS,            KC_PGDN,

        _______,  KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,      KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,              KC_ENT,             KC_HOME,

        _______,  KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,      KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,              KC_RSFT,  KC_UP,

        _______,  KC_LCTL,  KC_LOPT,  KC_LCMD,  MO(MAC_FN),         KC_SPC,                        KC_SPC,             KC_RCMD,  MO(MAC_FN), KC_RCTL,  KC_LEFT,  KC_DOWN,  KC_RGHT),



    [MAC_FN] = LAYOUT_91_ansi(

        RM_TOGG,  _______,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,     KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,     KC_F12,   _______,  _______,  RM_TOGG,

        _______,  _______,  _______,  _______,  _______,  _______,  _______,   _______,  _______,  _______,  _______,  _______,  _______,    _______,  _______,            _______,

        _______,  RM_TOGG,  RM_NEXT,  RM_VALU,  RM_HUEU,  RM_SATU,  RM_SPDU,   _______,  _______,  _______,  _______,  _______,  _______,    _______,  _______,            _______,

        _______,  _______,  RM_PREV,  RM_VALD,  RM_HUED,  RM_SATD,  RM_SPDD,   _______,  _______,  _______,  _______,  _______,  _______,              _______,            _______,

        _______,  _______,            _______,  _______,  _______,  _______,   _______,  NK_TOGG,  _______,  _______,  _______,  _______,              _______,  _______,

        _______,  _______,  _______,  _______,  _______,            _______,                       _______,            _______,  _______,    _______,  _______,  _______,  _______),



    [WIN_BASE] = LAYOUT_91_ansi(

        ENC_L,    KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,     KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,     KC_F12,   KC_INS,   KC_DEL,   KC_MUTE,

        _______,  KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,      KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,    KC_EQL,   KC_BSPC,            KC_PGUP,

        _______,  KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,      KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,    KC_RBRC,  KC_BSLS,            KC_PGDN,

        _______,  KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,      KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,              KC_ENT,             KC_HOME,

        _______,  KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,      KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,              KC_RSFT,  KC_UP,

        _______,  KC_LCTL,  KC_LWIN,  KC_LALT,  MO(WIN_FN),         KC_SPC,                        KC_SPC,             KC_RALT,  MO(WIN_FN), KC_RCTL,  KC_LEFT,  KC_DOWN,  KC_RGHT),



    [WIN_FN] = LAYOUT_91_ansi(

        RM_TOGG,  _______,  KC_BRID,  KC_BRIU,  KC_TASK,  KC_FLXP,  RM_VALD,   RM_VALU,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,    KC_VOLU,  _______,  _______,  RM_TOGG,

        _______,  _______,  _______,  _______,  _______,  _______,  _______,   _______,  _______,  _______,  _______,  _______,  _______,    _______,  _______,            _______,

        _______,  RM_TOGG,  RM_NEXT,  RM_VALU,  RM_HUEU,  RM_SATU,  RM_SPDU,   _______,  _______,  _______,  _______,  _______,  _______,    _______,  _______,            _______,

        _______,  _______,  RM_PREV,  RM_VALD,  RM_HUED,  RM_SATD,  RM_SPDD,   _______,  _______,  _______,  _______,  _______,  _______,              _______,            _______,

        _______,  _______,            _______,  _______,  _______,  _______,   _______,  NK_TOGG,  _______,  _______,  _______,  _______,              _______,  _______,

        _______,  _______,  _______,  _______,  _______,            _______,                       _______,            _______,  _______,    _______,  _______,  _______,  _______),

};



#if defined(ENCODER_MAP_ENABLE)

const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {

    [MAC_BASE] = { ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },

    [MAC_FN]   = { ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },

    [WIN_BASE] = { ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },

    [WIN_FN]   = { ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU) }

};

#endif // ENCODER_MAP_ENABLE


