# SDL3 迁移记录

日期：2026-09-27。优先交付 Android ARM64 与 macOS；iOS、Linux、Windows 保留公共接口与明确的平台待办，尚未宣称可运行。

本文件记录最初 0.2 迁移及当时产物哈希。之后的繁中字体修复、0.2.1 产物与实测证据见 [繁中修复记录](sdl3-zhtw-fix.md)；下方最初产物哈希不能作为当前文件的哈希。

## 代码结构

新后端由 `STUDYSTEADY_SDL3=ON` 选择，构建入口是 `cmake/sdl3_runtime.cmake`。默认选项与旧 Android 构建脚本保留，以兼容原有工具。依赖固定为 SDL 3.4.16，并记录来源与源码校验。

| 层 | 实现位置与职责 |
| --- | --- |
| 应用入口 | `apps/launcher/main.cpp`：SDL 主循环、游戏工作线程、资源目录、诊断入口与有序退出 |
| 公共平台 | `native/platform/sdl/`：窗口、主线程调度、输入、PCM、PNG/JPEG、路径、同步、内存信息和日志 |
| SDK 适配 | `tools/prepare_sdl_*.py`：在构建目录生成 SDK 头/源副本，保留原目录不变 |
| 游戏运行时 | `native/runtime/cotopha_port/legacy_*`：传统 Cotopha 对象解释器、原脚本与 Native 接口；SDL 分支复用这些实现 |
| E-mote | `native/extensions/emote/`：原 PSB/TJS/Player 与绘制，SDL 共享 GL context 和纹理寿命管理 |
| Android 外壳 | `apps/android/`：继承官方 SDLActivity；资源选择、导入、应用路径与启动参数 |
| macOS 包装 | `tools/build/build_sdl_desktop.py` 与 SDL macOS Info.plist：架构选择、Release 构建、临时目录签名、ZIP 复验 |

SDL 统一平台服务，绘制仍使用 EntisGLS 的 OpenGL/GLES 实现。macOS 使用 OpenGL 2.1，Android 目标使用 GLES 3。SDK 的 GL 能力检测、GLSL 版本、函数查找已按实际图形 API 适配；这不是 SDL_GPU 或 Metal 渲染器。

## 已解决的整合问题

- 主线程不等待脚本持有的 UI 锁；同步 GL 请求先入队，在能取得锁时执行。关闭窗口后仍服务资源释放请求。
- SDL 鼠标适配分开缓存更新与主动 warp。实际 trace 发现旧 `RememberPointer` 在 SDL 下反复移动系统指针，形成大量 motion 事件和坐标漂移；改为仅更新 SDK 缓存后，实际语言选择和标题 Start 都产生了正确命令，进入姓名选择页。显式 SDK 光标移动仍保留 warp。
- E-mote 持有共享窗口/context 的寿命引用，避免窗口先销毁、子渲染器后清理。实际共享纹理像素与宿主 GL 状态恢复通过。
- macOS 曾在 SDL Cocoa 的正 swap interval display-link 等待中卡住主线程。窗口改为即时交换，按约 60 Hz 判断绘制时机，隐藏/最小化/遮挡时不绘制和呈现；事件与资源任务继续处理。
- SDK 默认字体原本依赖平台字体接口，导致文字字形为空。为游戏注册 `Default → MsgFont`、`@Default → @MsgFont` 独立代理，使用游戏自带 BMF；不把未验证的 `FixedDefault` 冒充等宽字体。
- 音频使用 SDL AudioStream 消费位置与实际 PCM 队列；工作线程结束前释放 SDK TLS。等待音频线程时释放 SDK 全局锁，避免退出死锁。
- 资源目录与用户存档目录分离；`storage://game` 可映射任意已有资源目录。修复 SDK 新建目录权限和标准流重复关闭问题。
- ARM64 检测、NEON 源清单及指针尺寸适配完成。SDK 原先非 x86 的 32 位 `GLintptr/GLsizeiptr` 改为生成副本中的 `std::ptrdiff_t`。
- GLES 分支屏蔽 SDK 中 5 处不支持的 `GL_MULTISAMPLE` enable/disable；实际 NDK 预处理为 0 处，desktop 保持 5 处，双端语法验证通过。FBO 采样分配及 resolve 逻辑保留。

## 实际验证证据

| 检查 | 结果与证据 |
| --- | --- |
| 主程序运行时自测 | `build/sdl-macos-smoke/self-test-final.log`：退出 0，`Legacy standalone self-tests PASS`；包含传统对象/线程、真实资源、媒体状态、文字、存档序列化与失败回滚 |
| 最终 Intel ZIP 实测 | `artifacts/sdl3/macos-x86_64/runtime-verification.json`：最终 ZIP 解压到系统临时目录，严格验签后实际运行；使用包内 assets，完整运行时自测和真实 SDL 语言按钮命令都通过；测试记录绑定 ZIP 与可执行文件 SHA-256 |
| 默认字体 | 同一日志：18 个真实横/竖字形在 16/32/64px 有非零像素，与原 BMF 指标及像素一致；无效字号、缺字、短缓冲区与所有权检查通过 |
| 系统路径和文件操作 | `build/macos-sdl3-system-final.log`：Unicode/游戏路径映射、资源只读、读写/重命名/删除与系统服务通过 |
| 原版语言页面输入 | `build/sdl-macos-smoke/window-probe.log`：真实 SDL 鼠标事件得到 `ID_LANGPICKER_JA` 命令；输入到按钮到传统事件链通过 |
| 原版标题与 E-mote | `build/sdl-macos-smoke/title-run.log`、`start-dialog-second.log`：实际脚本进入标题，`nak_a.psb` 动画帧像素变化，正常退出。截图为 `artifacts/sdl3/macos-x86_64/title.png` |
| 最终鼠标整合 | `build/sdl-macos-smoke/start-input-fixed.log`：真实 SDL 鼠标事件先得到 `ID_LANGPICKER_JA`，再得到并消费 `ID_START`；截图 `start-input-fixed.png` 为实际姓名选择页，文字正常，进程退出 0 |
| 最终包新游戏首屏 | `build/sdl-macos-smoke/release-new-game.log`：最终 Intel ZIP 使用包内 assets，从语言选择、标题、姓名与确认进入原版 `common1_1`；`new-game.png` 已目视确认列车背景、角色名与日文正文，退出 0。记录 `new-game-run.json` 绑定最终 ZIP 哈希；测试存档隔离在临时目录 |
| 光标反馈回归 | `tests/unit/platform/test_sdl_input_cursor.cpp` 在真实 SDK/SDL 窗口中检查收到输入和逻辑移动仅更新缓存、显式 SDK 移动仍 warp；`sdl_input_cursor_probe.log` 通过，修复前临时对象的负向对照精确失败于收到输入不应 warp |
| 片头短回归 | `build/sdl-macos-smoke/opening-probe.log`：实际 MEI 两轮约 3 秒播放、帧推进、停止、关闭和重开通过；没有重新进行完整 97 秒片头或完整路线验证 |
| 窗口呈现 | `build/sdl-macos-smoke/presentation-smoke.log`：真实 AMD OpenGL 2.1 窗口隐藏/显示、最小化/恢复；隐藏/最小化不呈现，10 次同步 worker→main 回调完成 |
| GL 与共享纹理 | `build/sdl-motion-test/run.log`：实际 `haz_a.psb`，31,202 个非透明像素；共享像素、重复绘制、异常/销毁后宿主状态恢复通过 |
| 音频/图片 | `native/platform/sdl/tests/` 实际测试：PCM postmix 样本、队列/循环/暂停/退出、应用总音量；PNG 精确 RGBA、JPEG 误差与短写失败；dummy 音频测试不证明物理扬声器效果 |
| 同步 | `build/sdl-sync-test/thread-sanitizer.log`：真实等待/唤醒、地址隔离、竞争发布、80,000 次争用递增，TSan 通过 |
| 只读输入完整性 | `artifacts/sdl3/input-integrity.json`：6,242 项 PASS，含 SDK 1,805 文件；导入的 Motion/PSB/TJS 按提供时的字节快照校验，不声称其本身是原版上游 |

标题、鼠标菜单、姓名确认与第一段剧情首屏通过，不代表已验证完整剧情、系统姓名输入法或游戏菜单存读档。存档当前有真实文件与序列化自测证据；旧 Android JNI 的章节和冷读档记录不能直接移作本次 SDL 证据。

## 交付与复现

- Android：`artifacts/studysteady-sdl3-arm64-dev.apk`，详细记录为同名 `.build.json`。完整原生构建、APK 签名与 ELF 对齐检查通过；本次没有连接到手机，未安装运行。
- Intel macOS：`artifacts/sdl3/macos-x86_64/StudySteady.zip`，同目录 `build.json`。
- Apple Silicon macOS：`artifacts/sdl3/macos-arm64/StudySteady.zip`，同目录 `build.json`。完整交叉编译链接及解压后签名/架构检查通过，没有 Apple Silicon 运行证据。

本次最终包：

| 产物 | 字节数 | SHA-256 |
| --- | ---: | --- |
| Android APK | 54,424,387 | `23356b253301c20a9ac625d47b7833dc0ec9549bcd6f319b6cff2b24d2cb7835` |
| Intel macOS ZIP | 9,710,246 | `9e5d23108618b2113fff1a9c30a60e22fec608d215a90f15e87c1d6c91efdd11` |
| ARM64 macOS ZIP | 8,658,051 | `3631e23b1d91d281c3c95d0e147d81a36b18b318b1b847848bbb15c232b33fff` |

Android 包内三个 AArch64 ELF、16 KB LOAD/ZIP 对齐、APK CRC 与签名检查通过；证书与旧 APK 一致，旧 APK 字节未变，详见 `artifacts/sdl3/android-build-verification.json`。

macOS 最低 11.0；目前本地 ad-hoc 签名，未公证。签名在系统临时目录完成，再验证 ZIP 解压后的二进制哈希、签名与架构。Documents 中 `.app` 副本可能被 FileProvider 添加 FinderInfo，因此交付 ZIP。三个包都不含游戏 NOA。

构建：

```sh
python3 tools/build/build_sdl_android.py
python3 tools/build/build_sdl_desktop.py --arch x86_64
python3 tools/build/build_sdl_desktop.py --arch arm64
python3 tools/ci/verify_sdl_inputs.py
```

macOS 默认启动会选择并记住游戏目录。诊断可使用 `--game-dir`、`--local-dir` 与 `--storage-dir` 将测试保存隔离；`--self-test`、`--window-probe`、`--opening-probe` 为独立开发入口。`--exit-after` 是主循环中的正常退出请求，并非可以打断死锁的外部看门狗。

Android 安装后从应用启动页导入资源；已部署且应用确实可读的资源可复用。导入创建应用自己的新文件，逐个记录长度和 SHA-256、同步落盘，最后用带恢复日志的目录交换发布；旧资源保留为备份，存档不参与交换。完整原始资源约 10.8 GB，替换已有资源时需为备份和新副本预留空间。该导入流程已编译检查，手机上的 SAF provider 与中断恢复仍需实测。

## 后续平台范围

具体源级依据见 `docs/research/sdl3-platform-boundaries.md`。

- iOS 已单独选择 OpenGLES framework；仍需 GLES 头适配、UIKit 非零默认 framebuffer/renderbuffer、共享 context、沙箱资源导入、签名与前后台验证。
- Linux 复用 SDL/POSIX 路径；尚需真实 Linux 工具链、GL/GLU、X11/Wayland、音频及大小写敏感资源路径构建测试。
- Windows 仍需拆开传统兼容头与系统头、POSIX 锁/文件原子替换、动态符号查找、MSVC 编译参数及 LLP64/序列化边界。
- 录音、SDK 通用文件选择/进度对话框等未接入的非游戏路径显式返回不支持；macOS 应用自身的游戏目录选择使用 SDL 原生对话框。

这些差异集中在平台适配边界；仅把工程切换到另一平台的 CMake toolchain 还不能承诺编译和运行成功。
