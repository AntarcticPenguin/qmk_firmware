# 个人固件输出目录

编译好的 `.bin` 按**用途**分子文件夹（游戏 / 办公），再按键盘分子目录。

通用项目说明（左手用户偏好、工作流、编译环境）见根目录 `.cursor/rules/qmk-personal.mdc`。

## 我的键盘

| 用途 | 键盘 | 固件目录 | 说明 |
|------|------|----------|------|
| 游戏 | Keychron Q11 | `firmware/gaming/keychron_q11/` | keymap `gaming`；共用 `q11_rgb/` + `q11_shortcuts/` |
| 办公 | Keychron Q11 ANSI Encoder | `firmware/office/keychron_q11/` | keymap `default`；共用 `q11_rgb/` + `q11_shortcuts/` |

## 目录结构

```
firmware/
  gaming/                    ← 游戏用键盘
    keychron_q11/
      latest.bin
      archive/
  office/                    ← 办公用键盘
    keychron_q11/
      latest.bin             ← 刷机用这个（办公 Q11）
      archive/
```

源码和 keymap 仍在 `keyboards/` 下对应路径，本目录只存放可刷写的固件。
