#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct keyrecord_t keyrecord_t;

#define Q11_LEFT_SPC_ROW  5
#define Q11_LEFT_SPC_COL  6
#define Q11_RIGHT_SPC_ROW 11
#define Q11_RIGHT_SPC_COL 1

bool q11_shortcuts_process_record(uint16_t keycode, keyrecord_t *record);
void q11_shortcuts_matrix_scan(void);
