#pragma once

// keymap config.h 在 include 本文件前定义 Q11_RGB_PROFILE_GAMING 或 Q11_RGB_PROFILE_OFFICE

#if defined(Q11_RGB_PROFILE_GAMING)
#    define Q11_RGB_CYCLE_LIST Q11_RGB_OFF, Q11_RGB_SOLID
#elif defined(Q11_RGB_PROFILE_OFFICE)
#    define Q11_RGB_CYCLE_LIST Q11_RGB_OFF, Q11_RGB_SOLID, Q11_RGB_RIPPLE, Q11_RGB_WAVE, Q11_RGB_ZONE
#else
#    error "Define Q11_RGB_PROFILE_GAMING or Q11_RGB_PROFILE_OFFICE in keymap config.h"
#endif
