RGB_MATRIX_EFFECT(q11_static_solid)
RGB_MATRIX_EFFECT(q11_static_zone)
RGB_MATRIX_EFFECT(q11_typing_ripple)
RGB_MATRIX_EFFECT(q11_band_wave)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#    define Q11_RIPPLE_CENTER_MS  400
#    define Q11_RIPPLE_RING_DELAY 40
#    define Q11_RIPPLE_RING_MS    250
#    define Q11_RIPPLE_RING_PEAK  128

#    define Q11_BAND_OFFSET 152
#    define Q11_BAND_TRAVEL   256

void q11_rgb_render_static(uint8_t led_min, uint8_t led_max);

bool q11_static_solid(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    q11_rgb_render_static(led_min, led_max);
    return rgb_matrix_check_finished_leds(led_max);
}

bool q11_static_zone(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    q11_rgb_render_static(led_min, led_max);
    return rgb_matrix_check_finished_leds(led_max);
}

#    ifdef RGB_MATRIX_KEYREACTIVE_ENABLED

static bool q11_led_matrix_pos(uint8_t led_idx, uint8_t *row, uint8_t *col) {
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            if (g_led_config.matrix_co[r][c] == led_idx) {
                *row = r;
                *col = c;
                return true;
            }
        }
    }
    return false;
}

static uint8_t q11_matrix_manhattan(uint8_t r1, uint8_t c1, uint8_t r2, uint8_t c2) {
    return (r1 > r2 ? r1 - r2 : r2 - r1) + (c1 > c2 ? c1 - c2 : c2 - c1);
}

static uint8_t q11_ripple_fade(uint16_t elapsed, uint16_t duration, uint8_t peak) {
    if (elapsed >= duration) {
        return 0;
    }

    uint16_t remaining = duration - elapsed;
    uint8_t  linear    = (uint8_t)((remaining * 255) / duration);
    return scale8(scale8(linear, linear), peak);
}

bool q11_typing_ripple(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    const uint8_t global_val = rgb_matrix_config.hsv.v;
    hsv_t         theme_hsv  = rgb_matrix_config.hsv;

    uint8_t center_row[LED_HITS_TO_REMEMBER];
    uint8_t center_col[LED_HITS_TO_REMEMBER];
    bool    center_valid[LED_HITS_TO_REMEMBER];
    uint8_t hit_count = g_last_hit_tracker.count;

    for (uint8_t j = 0; j < hit_count; j++) {
        center_valid[j] = q11_led_matrix_pos(g_last_hit_tracker.index[j], &center_row[j], &center_col[j]);
    }

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        uint8_t  led_row;
        uint8_t  led_col;
        bool     led_mapped = q11_led_matrix_pos(i, &led_row, &led_col);
        uint16_t accum      = 0;

        for (uint8_t j = 0; j < hit_count; j++) {
            uint16_t elapsed    = g_last_hit_tracker.tick[j];
            uint8_t  center_led = g_last_hit_tracker.index[j];
            uint8_t  contrib    = 0;

            if (i == center_led) {
                if (elapsed < Q11_RIPPLE_CENTER_MS) {
                    contrib = q11_ripple_fade(elapsed, Q11_RIPPLE_CENTER_MS, UINT8_MAX);
                }
            } else if (center_valid[j] && led_mapped && q11_matrix_manhattan(led_row, led_col, center_row[j], center_col[j]) == 1) {
                if (elapsed >= Q11_RIPPLE_RING_DELAY) {
                    uint16_t ring_elapsed = elapsed - Q11_RIPPLE_RING_DELAY;

                    if (ring_elapsed < Q11_RIPPLE_RING_MS) {
                        contrib = q11_ripple_fade(ring_elapsed, Q11_RIPPLE_RING_MS, Q11_RIPPLE_RING_PEAK);
                    }
                }
            }

            accum += contrib;
        }

        if (accum > UINT8_MAX) {
            accum = UINT8_MAX;
        }

        if (accum == 0) {
            rgb_matrix_set_color(i, 0, 0, 0);
        } else {
            theme_hsv.v = scale8((uint8_t)accum, global_val);
            rgb_t rgb   = rgb_matrix_hsv_to_rgb(theme_hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#    endif // RGB_MATRIX_KEYREACTIVE_ENABLED

static int16_t q11_band_val_level(uint8_t x, uint8_t time, uint8_t val) {
    int16_t pos   = (int16_t)scale8(x, 228) + 28;
    int16_t delta = pos - (int16_t)time;
    int16_t level = (int16_t)val - abs(delta) * 8;

    return level < 0 ? 0 : level;
}

bool q11_band_wave(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    const uint8_t  val        = rgb_matrix_config.hsv.v;
    const uint16_t raw_time   = scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed / 4, 1));
    const uint16_t base_spawn = (raw_time / Q11_BAND_OFFSET) * Q11_BAND_OFFSET;

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        hsv_t   hsv   = rgb_matrix_config.hsv;
        uint8_t x     = g_led_config.point[i].x;
        int16_t level = 0;

        for (uint8_t w = 0; w < 2; w++) {
            uint16_t spawn = base_spawn - (uint16_t)w * Q11_BAND_OFFSET;

            if (spawn > raw_time) {
                break;
            }

            uint16_t age = raw_time - spawn;

            if (age >= Q11_BAND_TRAVEL) {
                continue;
            }

            int16_t l = q11_band_val_level(x, (uint8_t)age, val);

            if (l > level) {
                level = l;
            }
        }

        hsv.v     = (uint8_t)level;
        rgb_t rgb = rgb_matrix_hsv_to_rgb(hsv);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
