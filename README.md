# CH32H417 Logic Analyzer

[English](#english) · [中文](#中文)

开源逻辑分析仪固件，芯片为 **WCH CH32H417**。  
硬件开源页面：https://oshwhub.com/q2h2/project_bszkxrnf

上位机请用 **[ALL LOGIC](https://github.com/Doukeyi-X/ALL-LOGIC)**（在 DSView 上二次开发的非官方多厂商上位机）。

---

## 中文

本仓库只包含 **CH32H417 逻辑分析仪固件源码**（应用程序 + IAP），以及一份可直接烧录的预编译固件。  
不包含 ALL LOGIC / DSView 上位机源码。

### 目录

| 路径 | 说明 |
|------|------|
| `CH32H417_Logic_Analyzer_APP/` | 采集固件（V3F + V5F 双核工程） |
| `CH32H417_Logic_Analyzer_IAP/` | USB 升级 IAP |
| `SRC/` | WCH 外设库与启动文件 |
| `prebuilt/` | 预编译 `.bin` / `.hex`（V5F） |

请用 [MounRiver Studio](http://www.mounriver.com/) 打开 APP / IAP 的 `.wvsln` 工程编译。`SRC/` 来自南京沁恒（WCH）官方库，仅可用于沁恒单片机，文件头版权仍属于 WCH。

### 预编译固件

`prebuilt/` 里是已经编好的 V5F 镜像，可用 WCHISPTool / MounRiver 直接烧录：

| 文件 | 用途 |
|------|------|
| `CH32H417_Logic_Analyzer_V5F.hex` / `.bin` | 采集应用程序 |
| `CH32H417_IAP_V5F.hex` / `.bin` | USB IAP |

### 上位机

https://github.com/Doukeyi-X/ALL-LOGIC

### 许可证

- 本仓库中的逻辑分析仪应用 / IAP 代码：GNU GPLv3 或更高版本（见 `LICENSE`）
- `SRC/` 及带有 WCH 版权头的文件：遵循文件头中沁恒的许可（仅用于其 MCU）

### 给个 Star，也欢迎提 Issue

如果这个项目对你有用，请点一下右上角的 **Star**。

遇到烧录、采集或编译问题，请开 Issue：

https://github.com/Doukeyi-X/CH32H417-Logic-Analyzer/issues

### Contributors

| | |
|---|---|
| [Doukeyi-X](https://github.com/Doukeyi-X) | 开源整理与维护 |
| [Q2H2](https://oshwhub.com/q2h2/project_bszkxrnf) | 硬件与固件原作 |

---

## English

Open-source logic analyzer firmware for the **WCH CH32H417**.

Hardware project: https://oshwhub.com/q2h2/project_bszkxrnf  
Host software: **[ALL LOGIC](https://github.com/Doukeyi-X/ALL-LOGIC)** (unofficial secondary development of DSView).

This repository is **firmware only** (application + IAP) plus prebuilt V5F images. The ALL LOGIC / DSView host lives in a separate repo.

### Layout

| Path | Contents |
|------|----------|
| `CH32H417_Logic_Analyzer_APP/` | Capture firmware (V3F + V5F) |
| `CH32H417_Logic_Analyzer_IAP/` | USB IAP updater |
| `SRC/` | WCH peripheral library and startup |
| `prebuilt/` | Prebuilt `.bin` / `.hex` (V5F) |

Build the APP / IAP `.wvsln` projects with [MounRiver Studio](http://www.mounriver.com/). Files under `SRC/` and those with WCH headers are from Nanjing Qinheng and may only be used on WCH microcontrollers.

### Prebuilt images

Flash the V5F images in `prebuilt/` with WCHISPTool or MounRiver:

| File | Role |
|------|------|
| `CH32H417_Logic_Analyzer_V5F.hex` / `.bin` | Capture application |
| `CH32H417_IAP_V5F.hex` / `.bin` | USB IAP |

### License

- Application / IAP code in this repo: GNU GPLv3 or later (`LICENSE`)
- `SRC/` and WCH-headered files: follow the license in each file header

### Feedback

If this project helps you, please **Star** it.

Issues: https://github.com/Doukeyi-X/CH32H417-Logic-Analyzer/issues

### Contributors

| | |
|---|---|
| [Doukeyi-X](https://github.com/Doukeyi-X) | packaging and maintenance |
| [Q2H2](https://oshwhub.com/q2h2/project_bszkxrnf) | hardware and original firmware |
