/* Copyright 2023 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H

enum layers{
    MAC_BASE,
    MAC_FN,
    WIN_BASE,
    WIN_FN,
};

#define KC_TASK LGUI(KC_TAB)
#define KC_FLXP LGUI(KC_E)

// 右手 Home/PgUp/PgDn + 右 Ctrl 快捷键（左手用户，固定发 Windows Ctrl）
#define TAP_TERM_MS        200
#define REPEAT_DELAY_MS    350
#define REPEAT_INTERVAL_MS 45
#define ON_BASE_LAYER() (get_highest_layer(layer_state) == MAC_BASE || get_highest_layer(layer_state) == WIN_BASE)

typedef struct {
    bool     rctrl_held;
    bool     home_held;
    bool     home_chord;
    bool     home_mods_on;
    uint16_t home_timer;
    bool     pgdn_chord;
    uint16_t pgdn_timer;
    uint16_t pgup_timer;
    bool     left_held;
    bool     down_held;
    bool     repeat_undo;
    bool     repeat_redo;
    bool     repeat_ready;
    uint16_t repeat_timer;
} right_shortcut_state_t;

static right_shortcut_state_t right_state;

static void clear_home_mods(void) {
    if (right_state.home_mods_on) {
        unregister_mods(MOD_BIT_LCTRL | MOD_BIT_LSHIFT);
        right_state.home_mods_on = false;
    }
}

static void send_undo(void) {
    if (!right_state.home_mods_on) {
        register_mods(MOD_BIT_LCTRL);
        right_state.home_mods_on = true;
    }
    tap_code(KC_Z);
}

static void send_redo(void) {
    if (!right_state.home_mods_on) {
        register_mods(MOD_BIT_LCTRL | MOD_BIT_LSHIFT);
        right_state.home_mods_on = true;
    }
    tap_code(KC_Z);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!ON_BASE_LAYER()) {
        return true;
    }

    if (keycode == KC_RCTL) {
        right_state.rctrl_held = record->event.pressed;
        return true;
    }

    if (record->event.pressed && right_state.rctrl_held) {
        switch (keycode) {
            case KC_HOME:
                right_state.home_chord = true;
                tap_code16(LCTL(KC_X));
                return false;
            case KC_PGDN:
                right_state.pgdn_chord = true;
                tap_code16(LGUI(KC_V));
                return false;
            case KC_BSLS:
                tap_code16(LCTL(KC_E));
                return false;
            case KC_BSPC:
                tap_code16(LSFT(LCTL(KC_BSPC)));
                return false;
        }
    }

    switch (keycode) {
        case KC_HOME:
            if (record->event.pressed) {
                right_state.home_held  = true;
                right_state.home_chord = false;
                right_state.home_timer = timer_read();
            } else {
                uint16_t elapsed = timer_elapsed(right_state.home_timer);
                if (!right_state.home_chord && elapsed < TAP_TERM_MS) {
                    tap_code16(LCTL(KC_C));
                }
                right_state.home_held = false;
                right_state.repeat_undo = false;
                right_state.repeat_redo = false;
                right_state.repeat_ready = false;
                clear_home_mods();
            }
            return false;

        case KC_PGUP:
            if (record->event.pressed) {
                right_state.pgup_timer = timer_read();
            } else if (timer_elapsed(right_state.pgup_timer) < TAP_TERM_MS) {
                tap_code16(LCTL(KC_A));
            }
            return false;

        case KC_PGDN:
            if (record->event.pressed) {
                right_state.pgdn_chord = false;
                right_state.pgdn_timer = timer_read();
            } else if (!right_state.pgdn_chord && timer_elapsed(right_state.pgdn_timer) < TAP_TERM_MS) {
                tap_code16(LCTL(KC_V));
            }
            return false;

        case KC_LEFT:
            if (right_state.home_held) {
                if (record->event.pressed) {
                    right_state.home_chord = true;
                    right_state.left_held  = true;
                    right_state.repeat_undo = true;
                    right_state.repeat_ready = false;
                    right_state.repeat_timer = timer_read();
                    send_undo();
                } else {
                    right_state.left_held  = false;
                    right_state.repeat_undo = false;
                    if (!right_state.down_held) {
                        right_state.repeat_ready = false;
                        clear_home_mods();
                    }
                }
                return false;
            }
            break;

        case KC_DOWN:
            if (right_state.home_held) {
                if (record->event.pressed) {
                    right_state.home_chord = true;
                    right_state.down_held  = true;
                    right_state.repeat_redo = true;
                    right_state.repeat_ready = false;
                    right_state.repeat_timer = timer_read();
                    send_redo();
                } else {
                    right_state.down_held  = false;
                    right_state.repeat_redo = false;
                    if (!right_state.left_held) {
                        right_state.repeat_ready = false;
                        clear_home_mods();
                    }
                }
                return false;
            }
            break;
    }

    return true;
}

void matrix_scan_user(void) {
    if (!right_state.repeat_undo && !right_state.repeat_redo) {
        return;
    }

    if (!right_state.home_held) {
        return;
    }

    uint16_t elapsed = timer_elapsed(right_state.repeat_timer);
    uint16_t threshold = right_state.repeat_ready ? REPEAT_INTERVAL_MS : REPEAT_DELAY_MS;

    if (elapsed < threshold) {
        return;
    }

    right_state.repeat_timer = timer_read();
    right_state.repeat_ready = true;

    if (right_state.repeat_undo && right_state.left_held) {
        send_undo();
    } else if (right_state.repeat_redo && right_state.down_held) {
        send_redo();
    }
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [MAC_BASE] = LAYOUT_91_ansi(
        KC_MUTE,  KC_ESC,   KC_BRID,  KC_BRIU,  KC_MCTL,  KC_LPAD,  RM_VALD,   RM_VALU,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,    KC_VOLU,  KC_INS,   KC_DEL,   KC_MUTE,
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
        KC_MUTE,  KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,     KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,     KC_F12,   KC_INS,   KC_DEL,   KC_MUTE,
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
    [MAC_BASE] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [MAC_FN]   = { ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(RM_VALD, RM_VALU) },
    [WIN_BASE] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [WIN_FN]   = { ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(RM_VALD, RM_VALU) }
};
#endif // ENCODER_MAP_ENABLE
