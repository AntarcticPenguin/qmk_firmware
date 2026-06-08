# Q11 共用 RGB 配置

办公（`office_rgb/`）与游戏（`gaming_rgb/`）keymap 均 `#include` 本目录的 `rgb_config.h`。

## 作用

- 启用 `RGB_MATRIX_KEYPRESSES`（办公涟漪灯效需要）
- 禁用 QMK 内置动画（减小固件体积，避免与自定义灯效冲突）
- 统一默认亮度 / 速度等基线

## 灯效文档

| 用途 | 模块 | 说明 |
|------|------|------|
| 办公 | `office_rgb/README.md` | 五色模式、调色、办公彩分区 |
| 游戏 | `gaming_rgb/README.md` | 关闭 / 纯色 |

自定义效果实现见 `ansi_encoder/keymaps/default/rgb_matrix_user.inc`（办公）。
