# Keychron Q11 ANSI Encoder — 个人固件

## 文档索引

| 文档 | 内容 |
|------|------|
| `自定义说明.md` | 快捷键、编译刷机、USB 版本 |
| `快捷键规划.md` | 右手快捷键映射表 |
| `office_rgb/README.md` | 办公灯效模式与操作 |
| `gaming_rgb/README.md` | 游戏灯效模式与操作 |
| `base_rgb/README.md` | 共用 RGB 配置说明 |

## 编译

```powershell
.\compile.ps1          # 办公 → firmware/office/keychron_q11/
.\compile-gaming.ps1   # 游戏 → firmware/gaming/keychron_q11/
```

Q11 分体，刷机时左右两半各刷一次（左 Esc / 右 Del 进 DFU）。
