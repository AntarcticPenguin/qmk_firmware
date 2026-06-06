# 个人固件输出目录

编译好的 `.bin` 统一放在这里，按键盘分子文件夹，方便管理多块键盘。

通用项目说明（左手用户偏好、工作流、编译环境）见根目录 `.cursor/rules/qmk-personal.mdc`。

```
firmware/
  keychron_q11/          ← Keychron Q11
    latest.bin           ← 刷机用这个
    archive/             ← 历史版本
  <其他键盘>/            ← 以后新增
```

源码和 keymap 仍在 `keyboards/` 下对应路径，本目录只存放可刷写的固件。
