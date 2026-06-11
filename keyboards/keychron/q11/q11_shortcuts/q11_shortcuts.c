#include QMK_KEYBOARD_H

#include "../q11_rgb/q11_rgb.h"
#include "q11_keymap_defs.h"
#include "q11_shortcuts.h"

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
    bool     pgdn_held;
    bool     pgdn_chord;
    uint16_t pgdn_timer;
    uint16_t pgup_timer;
    uint16_t ins_timer;
    uint16_t del_timer;
    bool     left_held;
    bool     down_held;
    bool     repeat_undo;
    bool     repeat_redo;
    bool     repeat_ready;
    uint16_t repeat_timer;
    bool     enc_l_held;
    bool     enc_l_chord;
    uint16_t enc_l_timer;
} q11_shortcut_state_t;

static q11_shortcut_state_t shortcut_state;

static void clear_home_mods(void) {
    if (shortcut_state.home_mods_on) {
        unregister_mods(MOD_BIT_LCTRL | MOD_BIT_LSHIFT);
        shortcut_state.home_mods_on = false;
    }
}

static void send_undo(void) {
    if (!shortcut_state.home_mods_on) {
        register_mods(MOD_BIT_LCTRL);
        shortcut_state.home_mods_on = true;
    }
    tap_code(KC_Z);
}

static void send_redo(void) {
    if (!shortcut_state.home_mods_on) {
        register_mods(MOD_BIT_LCTRL | MOD_BIT_LSHIFT);
        shortcut_state.home_mods_on = true;
    }
    tap_code(KC_Z);
}

bool q11_shortcuts_process_record(uint16_t keycode, keyrecord_t *record) {
    if (keycode == ENC_L) {
        if (record->event.pressed) {
            shortcut_state.enc_l_held  = true;
            shortcut_state.enc_l_chord = false;
            shortcut_state.enc_l_timer = timer_read();
        } else {
            shortcut_state.enc_l_held  = false;
            shortcut_state.enc_l_chord = false;
        }
        return false;
    }

    if (!ON_BASE_LAYER()) {
        return true;
    }

    if (shortcut_state.enc_l_held) {
        switch (keycode) {
            case KC_LEFT:
            case KC_RGHT:
            case KC_UP:
            case KC_DOWN:
            case KC_PGUP:
            case KC_PGDN:
            case KC_INS:
            case KC_DEL:
                q11_rgb_process_enc_key(keycode, record, true);
                if (record->event.pressed) {
                    shortcut_state.enc_l_chord = true;
                }
                return false;
            case KC_SPC:
                if (record->event.pressed) {
                    shortcut_state.enc_l_chord = true;
                    if (record->event.key.row == Q11_LEFT_SPC_ROW && record->event.key.col == Q11_LEFT_SPC_COL) {
                        q11_rgb_cycle_mode_reverse();
                    } else if (record->event.key.row == Q11_RIGHT_SPC_ROW && record->event.key.col == Q11_RIGHT_SPC_COL) {
                        q11_rgb_cycle_mode();
                    }
                }
                return false;
            default:
                return false;
        }
    }

    if (keycode == KC_RCTL) {
        shortcut_state.rctrl_held = record->event.pressed;
        return true;
    }

    if (record->event.pressed && shortcut_state.rctrl_held) {
        switch (keycode) {
            case KC_HOME:
                shortcut_state.home_chord = true;
                tap_code16(LGUI(KC_V));
                return false;
            case KC_PGDN:
                shortcut_state.pgdn_chord = true;
                tap_code16(LCTL(KC_X));
                return false;
            case KC_UP:
                tap_code16(LCTL(KC_F));
                return false;
            case KC_RGHT:
                tap_code16(LSFT(LCTL(KC_F)));
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
                shortcut_state.home_held  = true;
                shortcut_state.home_chord = false;
                shortcut_state.home_timer = timer_read();
            } else {
                uint16_t elapsed = timer_elapsed(shortcut_state.home_timer);
                if (!shortcut_state.home_chord && elapsed < TAP_TERM_MS) {
                    tap_code16(LCTL(KC_V));
                }
                shortcut_state.home_held   = false;
                shortcut_state.repeat_undo = false;
                shortcut_state.repeat_redo = false;
                shortcut_state.repeat_ready = false;
                clear_home_mods();
            }
            return false;

        case KC_PGUP:
            if (record->event.pressed) {
                shortcut_state.pgup_timer = timer_read();
            } else if (timer_elapsed(shortcut_state.pgup_timer) < TAP_TERM_MS) {
                tap_code16(LCTL(KC_A));
            }
            return false;

        case KC_PGDN:
            if (record->event.pressed) {
                shortcut_state.pgdn_chord = false;
                shortcut_state.pgdn_held   = true;
                shortcut_state.pgdn_timer  = timer_read();
            } else {
                if (!shortcut_state.pgdn_chord && timer_elapsed(shortcut_state.pgdn_timer) < TAP_TERM_MS) {
                    tap_code16(LCTL(KC_C));
                }
                shortcut_state.pgdn_held = false;
            }
            return false;

        case KC_INS:
            if (q11_rgb_process_enc_key(keycode, record, shortcut_state.enc_l_held)) {
                if (record->event.pressed) {
                    shortcut_state.enc_l_chord = true;
                }
                return false;
            }
            if (record->event.pressed) {
                shortcut_state.ins_timer = timer_read();
            } else if (timer_elapsed(shortcut_state.ins_timer) < TAP_TERM_MS) {
                tap_code16(LSFT(LCTL(KC_ESC)));
            }
            return false;

        case KC_DEL:
            if (q11_rgb_process_enc_key(keycode, record, shortcut_state.enc_l_held)) {
                if (record->event.pressed) {
                    shortcut_state.enc_l_chord = true;
                }
                return false;
            }
            if (record->event.pressed) {
                shortcut_state.del_timer = timer_read();
            } else if (timer_elapsed(shortcut_state.del_timer) < TAP_TERM_MS) {
                tap_code16(LCTL(LALT(KC_A)));
            }
            return false;

        case KC_LEFT:
        case KC_RGHT:
        case KC_UP:
        case KC_DOWN:
            if (q11_rgb_process_enc_key(keycode, record, shortcut_state.enc_l_held)) {
                if (record->event.pressed) {
                    shortcut_state.enc_l_chord = true;
                }
                return false;
            }
            if (keycode == KC_LEFT && shortcut_state.pgdn_held) {
                if (record->event.pressed) {
                    shortcut_state.pgdn_chord = true;
                    tap_code16(LCTL(LALT(KC_LEFT)));
                }
                return false;
            }
            if (keycode == KC_RGHT && shortcut_state.pgdn_held) {
                if (record->event.pressed) {
                    shortcut_state.pgdn_chord = true;
                    tap_code16(LCTL(LALT(KC_RGHT)));
                }
                return false;
            }
            if (keycode == KC_LEFT && shortcut_state.home_held) {
                if (record->event.pressed) {
                    shortcut_state.home_chord   = true;
                    shortcut_state.left_held    = true;
                    shortcut_state.repeat_undo  = true;
                    shortcut_state.repeat_ready = false;
                    shortcut_state.repeat_timer = timer_read();
                    send_undo();
                } else {
                    shortcut_state.left_held   = false;
                    shortcut_state.repeat_undo = false;
                    if (!shortcut_state.down_held) {
                        shortcut_state.repeat_ready = false;
                        clear_home_mods();
                    }
                }
                return false;
            }
            if (keycode == KC_DOWN && shortcut_state.home_held) {
                if (record->event.pressed) {
                    shortcut_state.home_chord   = true;
                    shortcut_state.down_held    = true;
                    shortcut_state.repeat_redo  = true;
                    shortcut_state.repeat_ready = false;
                    shortcut_state.repeat_timer = timer_read();
                    send_redo();
                } else {
                    shortcut_state.down_held   = false;
                    shortcut_state.repeat_redo = false;
                    if (!shortcut_state.left_held) {
                        shortcut_state.repeat_ready = false;
                        clear_home_mods();
                    }
                }
                return false;
            }
            break;
    }

    return true;
}

void q11_shortcuts_matrix_scan(void) {
    if (!shortcut_state.repeat_undo && !shortcut_state.repeat_redo) {
        return;
    }

    if (!shortcut_state.home_held) {
        return;
    }

    uint16_t elapsed     = timer_elapsed(shortcut_state.repeat_timer);
    uint16_t threshold   = shortcut_state.repeat_ready ? REPEAT_INTERVAL_MS : REPEAT_DELAY_MS;

    if (elapsed < threshold) {
        return;
    }

    shortcut_state.repeat_timer = timer_read();
    shortcut_state.repeat_ready = true;

    if (shortcut_state.repeat_undo && shortcut_state.left_held) {
        send_undo();
    } else if (shortcut_state.repeat_redo && shortcut_state.down_held) {
        send_redo();
    }
}
