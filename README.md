# OpenSourceLogic-CH32H417

[English](#english) · [中文](#中文)

本仓库是 **CH32H417 USB3.0 逻辑分析仪** 的开源固件（IAP + APP）。  
要和 **[ALL LOGIC](https://github.com/Doukeyi-X/ALL-LOGIC)** 上位机一起用，**必须先烧录本仓库的固件**。IAP 烧进去之后，以后可以用 ALL LOGIC 直接升级 APP。

---

## 中文

### 项目来源

硬件和原始固件来自立创开源平台 **Q2H2** 的工程：

**[USB3.0逻辑分析仪-16通道200MHz-CH32H417版](https://oshwhub.com/q2h2/project_bszkxrnf)**（GPL 3.0）

原项目是沁恒 **CH32H417** 单芯片方案：16 通道、最高 200 MHz 连续采样，USB 3.0（5 Gbps），另带 2 通道最高 20 Msps 模拟采样。原配套上位机叫 **U3LogicAnalyzer**，在开源软件 [PulseView](https://sigrok.org) 上改过。PCB、3D 外壳、亚克力面板仍以立创工程为准。

本仓库**不是**从零写的另一套固件，而是把上述 MCU 工程（APP + IAP）单独整理出来，方便和 **[ALL LOGIC](https://github.com/Doukeyi-X/ALL-LOGIC)** 一起用。ALL LOGIC 是在 DreamSourceLab **[DSView](https://github.com/DreamSourceLab/DSView)** 上做的二次开发（DSView 本身也源于 PulseView）。

| 部分 | 在哪 |
|------|------|
| 硬件、外壳、原工程说明 | [立创开源页](https://oshwhub.com/q2h2/project_bszkxrnf) |
| 本仓库 | IAP + APP 固件源码和预编译镜像 |
| 上位机 | [ALL LOGIC](https://github.com/Doukeyi-X/ALL-LOGIC) |

本仓库不包含 ALL LOGIC / DSView 源码，也不是沁恒或梦源官方产品。

### 必须烧本仓库固件才能用上位机

ALL LOGIC 按本仓库这套 APP / IAP 的 USB 协议和升级流程对接。

- 立创附件里的原固件、原 **U3LogicAnalyzer**，**不能**直接拿来当 ALL LOGIC 设备用
- 反过来，烧了本仓库固件后，请用 **ALL LOGIC**，不要再用原 U3LogicAnalyzer
- 新板或空片：先用调试器烧本仓库的 **IAP**，再用 ALL LOGIC 烧 **APP**
- 已经有本仓库 IAP 的板子：只需用 ALL LOGIC 升级 APP

烧的不是这份固件，上位机认不到设备，也升不了级。

### 烧录和升级（与原开源页同一套流程）

原工程说明里是两步：调试器先下 IAP，再用上位机「固件升级」下 APP。这里相同，只是上位机换成 ALL LOGIC。

需要自备 **WCH-LinkE**。下载口是芯片左侧排针 **H1**：从上往下，第 1 脚 SWCLK，第 2 脚 SWDIO。板子没把 ISP 用的 USB/串口引出来；若要用 USB/串口 ISP，需飞线（USB：PA11/PA12，串口：PB6/PB7）。

**第一次（空片 / 还没有本仓库 IAP）**

1. 安装 [MounRiver Studio](http://www.mounriver.com/)，或直接用 `prebuilt/` 里编好的镜像。
2. 打开 `CH32H417_Logic_Analyzer_IAP/CH32H417_IAP.wvsln`，编译后得到 IAP；也可直接用 `prebuilt/CH32H417_IAP_V5F.bin`。若编译报 `No rule to make target`，对工程点 Clean 再编。
3. MRS 菜单 **Tools → WCH-LinkUtility**，用 WCH-LinkE 把 **IAP** 下到芯片。
4. 用 Type-C 把分析仪接到电脑，打开 **[ALL LOGIC](https://github.com/Doukeyi-X/ALL-LOGIC)**。
5. 点工具栏 **「固件升级」**，选择本仓库的 APP：`prebuilt/CH32H417_Logic_Analyzer_V5F.bin`（或自己编出来的 APP `.bin`）。  
   这和原 U3LogicAnalyzer 的「帮助 → 固件升级」是同一类操作。

APP 下完即可在 ALL LOGIC 里采集。

**以后升级 APP**

不必再插 WCH-Link。USB 连上 ALL LOGIC，再点 **「固件升级」**，选新的 APP `.bin` 即可。通用上位机可以直接升级本仓库固件。

### 预编译镜像

| 文件 | 用途 | 怎么下 |
|------|------|--------|
| `prebuilt/CH32H417_IAP_V5F.bin` / `.hex` | IAP（第一次） | WCH-Link / WCHISPTool |
| `prebuilt/CH32H417_Logic_Analyzer_V5F.bin` / `.hex` | APP（采集） | ALL LOGIC「固件升级」 |

### 自己编译

用 MounRiver Studio 打开：

- IAP：`CH32H417_Logic_Analyzer_IAP/CH32H417_IAP.wvsln`
- APP：先编 V3F，再编 V5F：`CH32H417_Logic_Analyzer_APP/CH32H417_Logic_Analyzer.wvsln`

`SRC/` 是南京沁恒官方库，只能用于沁恒 MCU。V5F 还要链接 `CH32H417_Logic_Analyzer_APP/Common/ch32h417_uhsif_it.o`（沁恒 UHSIF 预编译库，无对应源码，仓库里已带上）。

### 目录

| 路径 | 说明 |
|------|------|
| `CH32H417_Logic_Analyzer_APP/` | 采集固件（V3F + V5F） |
| `CH32H417_Logic_Analyzer_IAP/` | USB 升级 IAP |
| `SRC/` | WCH 外设库与启动文件 |
| `prebuilt/` | 预编译 `.bin` / `.hex` |

### 许可证

- 应用 / IAP：GNU GPLv3 或更高版本（见 `LICENSE`），与立创原工程一致
- `SRC/` 及带 WCH 版权头的文件：按文件头，仅用于沁恒 MCU
- `ch32h417_uhsif_it.o`：沁恒 UHSIF 预编译对象，无源码，仅可用于 CH32H417

### 给个 Star，也欢迎提 Issue

有用的话请点右上角 **Star**。烧录、升级或采集有问题请开 Issue：

https://github.com/Doukeyi-X/OpenSourceLogic-CH32H417/issues

上位机问题请到 [ALL LOGIC Issues](https://github.com/Doukeyi-X/ALL-LOGIC/issues)。

### Contributors

| | |
|---|---|
| [Doukeyi-X](https://github.com/Doukeyi-X) | 开源整理、与 ALL LOGIC 对接 |
| [Q2H2](https://oshwhub.com/q2h2/project_bszkxrnf) | 硬件与固件原作 |

---

## English

Firmware (IAP + APP) for the **CH32H417 USB 3.0 logic analyzer**.  
**You must flash the images from this repository** before [ALL LOGIC](https://github.com/Doukeyi-X/ALL-LOGIC) will talk to the device. After IAP is on the chip, ALL LOGIC can upgrade the APP over USB.

### Origin

Hardware and original firmware come from **Q2H2** on OSHWHUB:

**[USB 3.0 logic analyzer, 16 ch / 200 MHz, CH32H417](https://oshwhub.com/q2h2/project_bszkxrnf)** (GPL 3.0)

The original project is a single-chip **CH32H417** design: 16 channels at up to 200 MHz continuous capture over USB 3.0 (5 Gbps), plus 2-channel analog sampling up to 20 Msps. The original host was **U3LogicAnalyzer**, a modified [PulseView](https://sigrok.org). PCB, 3D case, and acrylic panel stay on the OSHWHUB page.

This repository is **not** a from-scratch firmware. It packages that MCU tree (APP + IAP) so it can be used with **[ALL LOGIC](https://github.com/Doukeyi-X/ALL-LOGIC)**, which is secondary development on DreamSourceLab **[DSView](https://github.com/DreamSourceLab/DSView)** (itself based on PulseView).

| Piece | Where |
|------|------|
| Hardware, case, original write-up | [OSHWHUB project](https://oshwhub.com/q2h2/project_bszkxrnf) |
| This repo | IAP + APP source and prebuilt images |
| Host | [ALL LOGIC](https://github.com/Doukeyi-X/ALL-LOGIC) |

This repo does not contain ALL LOGIC / DSView sources. It is not an official WCH or DreamSourceLab product.

### Flash this firmware or the host will not work

ALL LOGIC speaks the USB protocol and IAP sequence of **this** APP / IAP.

- The stock images and **U3LogicAnalyzer** from the OSHWHUB attachments will **not** work as an ALL LOGIC device
- After flashing this repo, use **ALL LOGIC**, not U3LogicAnalyzer
- Blank chip: debugger-flash **IAP** from this repo, then use ALL LOGIC to load **APP**
- Board that already has this IAP: upgrade APP from ALL LOGIC only

Wrong firmware → the host will not enumerate the analyzer or upgrade it.

### Flash and upgrade (same flow as the original project)

The OSHWHUB notes use a debugger for IAP, then the host **Firmware Upgrade** for APP. Same here; the host is ALL LOGIC.

You need a **WCH-LinkE**. Connector **H1** on the left of the MCU: pin 1 SWCLK, pin 2 SWDIO from the top. ISP USB/UART are not brought out; fly-wire PA11/PA12 (USB) or PB6/PB7 (UART) if you need WCH ISP.

**First time (blank, or no IAP from this repo)**

1. Install [MounRiver Studio](http://www.mounriver.com/), or use `prebuilt/`.
2. Build `CH32H417_Logic_Analyzer_IAP/CH32H417_IAP.wvsln`, or use `prebuilt/CH32H417_IAP_V5F.bin`. If make says `No rule to make target`, Clean the project and rebuild.
3. **Tools → WCH-LinkUtility**, download **IAP** with WCH-LinkE.
4. Plug the analyzer in over Type-C and open **[ALL LOGIC](https://github.com/Doukeyi-X/ALL-LOGIC)**.
5. Toolbar **Firmware Upgrade**, pick this repo’s APP: `prebuilt/CH32H417_Logic_Analyzer_V5F.bin` (or your own APP `.bin`).  
   Same idea as U3LogicAnalyzer **Help → Firmware Upgrade**.

After APP is on, capture in ALL LOGIC.

**Later APP updates**

No debugger. Connect USB, open ALL LOGIC, **Firmware Upgrade**, choose the new APP `.bin`. The generic host upgrades this firmware directly.

### Prebuilt images

| File | Role | How to load |
|------|------|-------------|
| `prebuilt/CH32H417_IAP_V5F.bin` / `.hex` | IAP (first time) | WCH-Link / WCHISPTool |
| `prebuilt/CH32H417_Logic_Analyzer_V5F.bin` / `.hex` | APP (capture) | ALL LOGIC Firmware Upgrade |

### Build

MounRiver Studio:

- IAP: `CH32H417_Logic_Analyzer_IAP/CH32H417_IAP.wvsln`
- APP: build V3F, then V5F: `CH32H417_Logic_Analyzer_APP/CH32H417_Logic_Analyzer.wvsln`

`SRC/` is Nanjing Qinheng’s library (WCH MCUs only). V5F also links `CH32H417_Logic_Analyzer_APP/Common/ch32h417_uhsif_it.o` (WCH UHSIF object, no source; shipped here).

### Layout

| Path | Contents |
|------|----------|
| `CH32H417_Logic_Analyzer_APP/` | Capture firmware (V3F + V5F) |
| `CH32H417_Logic_Analyzer_IAP/` | USB IAP |
| `SRC/` | WCH peripheral library and startup |
| `prebuilt/` | Prebuilt `.bin` / `.hex` |

### License

- Application / IAP: GNU GPLv3 or later (`LICENSE`), same as the OSHWHUB project
- `SRC/` and WCH-headered files: file-header license, WCH MCUs only
- `ch32h417_uhsif_it.o`: WCH UHSIF object, no source, CH32H417 only

### Feedback

Please **Star** the repo if it helps.

Firmware issues: https://github.com/Doukeyi-X/OpenSourceLogic-CH32H417/issues  
Host issues: [ALL LOGIC Issues](https://github.com/Doukeyi-X/ALL-LOGIC/issues)

### Contributors

| | |
|---|---|
| [Doukeyi-X](https://github.com/Doukeyi-X) | packaging and ALL LOGIC integration |
| [Q2H2](https://oshwhub.com/q2h2/project_bszkxrnf) | hardware and original firmware |
