// 个人固件：USB 版本与原厂 Keychron Launcher 对齐（原厂 bin 显示 1.0.1）
#pragma once

#undef DEVICE_VER
#define DEVICE_VER 0x0101

#define Q11_RGB_PROFILE_OFFICE
#include "../../../q11_rgb/rgb_config.h"

// 快速连打时保留更多涟漪叠加
#define LED_HITS_TO_REMEMBER 16
