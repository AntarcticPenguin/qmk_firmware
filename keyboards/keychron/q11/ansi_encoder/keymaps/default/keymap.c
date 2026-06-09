/* Copyright 2023 @ Keychron (https://www.keychron.com) */
#include QMK_KEYBOARD_H

#include "../../../q11_rgb/q11_rgb.h"
#include "../../../q11_shortcuts/q11_keymap_defs.h"
#include "../../../q11_shortcuts/q11_shortcuts.h"

#include "../keymap.inc"

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return q11_shortcuts_process_record(keycode, record);
}

void keyboard_post_init_user(void) {
    keyboard_post_init_q11_rgb();
}

void matrix_scan_user(void) {
    q11_shortcuts_matrix_scan();
}
