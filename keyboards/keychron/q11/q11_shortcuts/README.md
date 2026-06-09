# Q11 共用快捷键

办公 / 游戏 keymap 共用 `q11_shortcuts.c`；改一处，两块键盘重新编译即同步。

## 文件

| 文件 | 作用 |
|------|------|
| `q11_shortcuts.c` | 右手快捷键、左旋钮灯效组合键 |
| `q11_keymap_defs.h` | layer 枚举、`ENC_L` 等共用定义 |

键位布局在 `ansi_encoder/keymaps/keymap.inc`，两个 keymap 均 `#include` 同一份。

## 差异仅在于

| keymap | `config.h` |
|--------|------------|
| `default` | `Q11_RGB_PROFILE_OFFICE`（5 种灯效循环） |
| `gaming` | `Q11_RGB_PROFILE_GAMING`（2 种灯效循环） |

除非在需求里明确「游戏专用 / 办公专用」，否则不要拆成两套快捷键逻辑。
