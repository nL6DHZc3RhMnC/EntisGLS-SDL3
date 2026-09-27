# SDL3 平台边界与剩余工作

审计日期：2026-09-27。本文件记录 SDL 迁移的代码和验证边界；构建、打包与运行是分别记录的证据。当前已完整构建 Android ARM64、macOS Intel 和 macOS ARM64，原游戏运行验证发生在 Intel macOS。iOS、Linux、Windows 尚未完成目标构建或运行验证。

## 已经形成的公共边界

`cmake/sdl3_runtime.cmake` 的 SDL 目标从官方 `EntisGLS/` 取源；`tools/sdk/prepare_sdl_sdk.py` 生成头文件和源文件适配副本。SDK 原目录保持只读。SDL 后端不编译原 Android Java/JNI 窗口、AudioTrack 或 Bitmap 实现。

| 功能 | 当前实现 | 仍需遵守的语义 |
|---|---|---|
| 窗口、键盘、触控、鼠标、前后台 | SDL 窗口与事件；生成的 `SGLGenericWindow` 继续连接 SDK handler 和 view framework | 主线程不能等待脚本持有的 UI 锁；待处理事件和渲染任务必须继续排空 |
| 渲染 | SDK 的 OpenGL/GLES 绘制逻辑、SDL context 和 proc lookup | SDL 没有自动把现有绘制改写为 SDL_GPU/Metal；每个目标仍需验证实际 GL 能力 |
| E-mote | 保留 PSB/TJS/Player；SDL 主线程串行执行并使用独立共享 GL context | 调用后恢复宿主 context；逻辑关窗与物理 GL window 的寿命分开，后台恢复仍需逐平台验证 |
| 音频 | SDK 解码器 + SDL AudioStream PCM 输出 | 位置来自消费帧；流式循环仍由 SDK decoder 管理；回调不能持 SDL 音频锁解码 |
| 总音量 | 所有当前和未来 PCM stream 的统一应用 master gain | SDL 分支不修改系统全局音量；设置失败不发布部分成功的值 |
| PNG/JPEG | SDL 3.4 PNG IO + 私有静态 stb JPEG + SDK SFile 桥 | PNG 保留直通 alpha，JPEG 丢弃 alpha；短写、格式和尺寸错误必须失败 |
| ERI/BMP/MEI/MIO、`.bmf` 字体 | 保留原 SDK 实现；游戏 `Default/@Default` 使用 `MsgFont/@MsgFont` 的独立代理 | 位图字号缩放、metrics 和像素由原 BMF 实现提供；未实现通用系统字体或任意格式媒体后端 |
| 系统服务 | SDL 日志、时间、CPU、资源根目录；可移植等待实现 | 内存可用量等 SDL 不提供的字段仍需小型 OS 查询，未知字段不能编造 |

SDL3 固定源版本为 3.4.16。PNG IO API 在该版本实际存在；JPEG 不是 SDL 核心提供的功能。`image_codec.cpp` 将 stb 符号设为内部链接，避免与 SDK tinygltf 的实现重复定义。SDL 3.4.16 PNG writer 对非零短写的判断不足，适配器通过先编码到内存、再校验目标精确写入长度处理此问题。

## 各平台的实际状态

| 平台 | 本次可确认的证据 | 尚未由这些证据证明的内容 |
|---|---|---|
| Android ARM64 | SDL 官方 Java 引导、传统运行时、E-mote 与 SDK 完整编译链接；APK 完整打包签名，记录了三个原生库和 16KB ELF 对齐；见 `artifacts/studysteady-sdl3-arm64-dev.build.json` | 新 SDL APK 尚未真机安装运行；资源导入、手机 GPU/音频、触控、前后台与旧存档仍需设备验证 |
| macOS x86_64 | 最终 ZIP 解压后实际运行：语言选择、标题、姓名确认、第一段剧情与日文正文；SDL 鼠标输入和 GL 截图；原 opening.mei 两轮短播放/停止/关闭/重新打开；最终包完整 legacy 自测退出 0 | 不是全剧情验收；长电影完整播放、系统姓名输入法、游戏菜单存读档、长时间运行及系统休眠恢复未由这些测试覆盖 |
| macOS ARM64 | 官方 SDK、传统运行时、E-mote 和 SDL 应用完整编译链接成功；`file`/`lipo` 确认 arm64；ZIP CRC 与解压后 ad-hoc 签名严格验证通过 | Apple Silicon 机器上的图形、音频、游戏与应用生命周期运行验证 |
| iOS | 公共 PCM、图片、主线程事件和资源接口可复用；CMake 已为 iOS 单独选择 GLES 与 OpenGLES framework | 尚无 iOS SDK 构建/签名/设备验证；GLES 头文件、UIKit 屏幕 framebuffer/renderbuffer、应用入口与打包、文件导入仍需实现或适配 |
| Linux | 公共 SDL/POSIX 方向相符，Linux 内存查询存在实现 | 未配置或编译 Linux 目标，未验证 X11/Wayland、GL 驱动、音频和大小写敏感路径 |
| Windows | 公共 PCM/图片核心使用 SDL/C++，未加入新的 Win32 依赖 | 整个 SDK/传统运行时仍有 Windows 宏、POSIX、类型和编译器分支阻碍，不能直接宣布 MSVC/MinGW 可用 |

旧 Android JNI APK 的运行记录不能作为新 SDL 目标的运行验证。独立 dummy 音频测试真实执行 AudioStream 转换、设备调度和 postmix 检查，但不证明物理扬声器输出。macOS 的进程、绘制和解码日志也不等同于人工听辨音频质量。

## 已解决的整合问题

- SDK 桌面 GL2.1 原先误查 `GL_MAJOR_VERSION`、GLES 的 `*_VECTORS` 限额以及 program binary 枚举；`prepare_sdl_graphics.py` 现在解析合法版本字符串、将桌面 component 数转换为 vec4 数，并按实际 API 能力查询 image units/program binary。GL2.1 的普通 shader 使用 GLSL120，属性 instancing 仍保留；依赖内建 vertex/instance ID 的 VT 分支按语言能力启用。GLES3.0 使用 GLSL300，不误要求310；geometry shader 需要相应 core 语言版本。Intel 后续真实标题运行通过，Android 的 GLES 驱动行为仍待真机验证。
- `GameFontAlias` 注册独立的 `Default/@Default` stock generator，持有会失效的原字体引用，每次请求生成独立样式对象；没有把同一个 owning pointer 登记到多个 key。18 个真实 BMF 横竖 glyph 在16/32/64px 的 metrics、像素与原 `MsgFont/@MsgFont` 完全一致，并验证了非零像素、缺字、短缓冲区、零字号和源对象销毁。SDK overlay 同时修复了默认字体 ownership 未初始化与 stock 创建失败后二次解锁。原 Message/SpriteDraw/SpriteState 三项失败全部通过，日志为 `build/sdl-font-check/self-test.log`。BMF 的字形覆盖和样式能力保持其原有范围；这不是通用系统字体实现。游戏 CSX 未发现 `FixedDefault`，SDK console 会请求它，但不能把尚未证明等宽的 `MsgFont` 冒充等宽字体。
- E-mote 的主线程派发在脚本线程等待前释放并恢复 SDK 全局/UI 锁，主线程使用非阻塞尝试，避免 dispatcher 被反向等待卡住。窗口 lease 让共享 GL 子 context 保留物理 window；正常关闭先解除 SDK/输入/显示关系，再在最后一个持有者释放时销毁 GL。真实 E-mote 独立测试验证了共享纹理、重复绘制像素和宿主 context/state 恢复；窗口 lease 测试验证了逻辑关闭与最终物理释放。少见的异常关闭、手机后台恢复不能仅凭这些独立测试宣称全部覆盖。
- SDL PCM feeder 在线程结束前调用 SDK TLS 清理，`Close` 返回前等待其完成。SDK wrapper 的 `Open/Close/SetListener` 等待回调期间释放并恢复 SDK 锁，防止回调等待全局锁、调用方同时等待回调。真实 dummy 音频测试验证了 finalizer 在线程内运行且 join 返回前完成。

## 需要优先解决的具体阻碍

### Android 与所有 ARM64 目标

SDK 的 `sgl3d_matrix.cpp` 和 `sglx3d_collision.cpp` 在 ARM 宏分支调用位于 `neon/` 子目录的实现。普通 `*.cpp` glob 不会纳入这些源；只在运行时关闭 NEON capability 也不能移除链接时引用。

需要纳入：

- `Source/common/sakuragl/sgl3d/neon/sgl3d_matrix_neon.cpp`
- `Source/common/sakuraglx/render/neon/sglx3d_collision_neon.cpp`

已反馈主构建并加入这两个源。其源内的 ARM 条件编译允许在 x86 构建中不产生 NEON 代码。不要再用全局伪造 `__arm__=1` 来决定 SDL 后端的 CPU 类型。

Android 已提供 SDL 官方 Activity/Java 引导、Manifest、资源打包与签名流程，完整 APK 构建通过。`libmain.so`、SDL Java 类和 `libSDL3.so` 使用同一固定版本。剩余项目 Java 代码负责资源选择、导入和路径传递：SAF 选取后复制到应用自己创建的文件，按实际 app IO 验证可读性并保留旧目录备份。此路径尚未在新 SDL APK 真机验证。SDL 并不免除 Android 存储访问规则，原手动复制文件的权限问题不能用桌面或编译结果代替验收。

### iOS 图形与应用入口

`cmake/sdl3_runtime.cmake` 已在通用 `APPLE` 分支前单独处理 iOS，链接 `OpenGLES` framework；macOS 继续链接 `OpenGL`。这只解决了链接配置中的一项已知错误，并不表示 iOS 目标已编译。

生成的 SDK GLES 头仍使用 Android/Linux 风格的 `<GLES/...>`、`<GLES2/...>`、`<GLES3/...>` 路径；`native/platform/gl.h` 的 SDL 分支使用 desktop `SDL_opengl.h`。应让图形头选择显式跟随 `STUDYSTEADY_GL_ES`，并用 SDL GLES 声明或专用的 GLES header overlay，避免将操作系统宏当成图形 API 宏。

UIKit 的屏幕 framebuffer 不能假定为0。固定 SDL 源的 `SDL_video.h` 明确要求绘制时绑定 `SDL_PROP_WINDOW_UIKIT_OPENGL_FRAMEBUFFER_NUMBER`，swap 时绑定相应 `SDL_PROP_WINDOW_UIKIT_OPENGL_RENDERBUFFER_NUMBER`，使用 MSAA 时还涉及 resolve framebuffer。当前 SDK `AttachFrameBuffer(nullptr)` 仍绑定0，部分 blit 收尾也恢复到0；必须把“宿主屏幕目标”作为平台值传递和恢复，并验证尺寸变化、E-mote context 切换及 swap。仅替换 framework 或编译通过不会解决这个运行时边界。

SDK desktop OpenGL 代码仍引用 `gluErrorString`、`gluBuild2DMipmaps`；这两个调用点已经由 `!defined(__API_OPEN_GL_ES__)` 保护，当前移动目标定义该宏后不会编译这些调用。因此 GLU 目前是 desktop 链接依赖，不能误写成已确认的 iOS 绘制阻碍。生成 `GL/glu.h` 声明本身不提供实现；后续仍应保持 GLES 不走 GLU。

随后还需完成 `.app` bundle/Info.plist、设备与模拟器架构、签名、横屏与安全区、前后台/音频中断、资源选择及沙箱访问。当前 `sdl_main.cpp` 的桌面 `--game-dir` 路径方案并不构成 iOS 文件导入 UI。

### macOS ARM64 与现代编译器

原 SDK 的 ARM64 检测外层要求 `__arm__`，Apple ARM64 编译器不需要定义这个宏。生成 overlay 已扩展为识别 `__aarch64__`/`__arm64__`；实际预处理检查确认 `__POINTER64__=1`、`__PROCESSOR_ARM64__=8`、`__PROCESSOR_ARM__=7`。

AppleClang 21 的 C++17 `<atomic>` 不兼容原 Unix 头带入的 C `<stdatomic.h>` 宏；overlay 已切换到 C++ `<atomic>` 并引入所需 memory-order 常量，同时补上 `va_list` 的标准头。这是编译边界修复，不是游戏状态/脚本语义变更。

ARM64 完整编译和链接已在 Intel macOS 主机上交叉构建完成，最低部署版本为 macOS 11.0。此主机没有运行 ARM64 程序；x86_64 的 OpenGL 验证不能替代 ARM64 GPU/驱动、线程退出以及存档反序列化测试。

`tools/build/build_sdl_desktop.py` 在系统临时目录中签名生成副本，仅清除会阻止签名的 `com.apple.FinderInfo` 与 `com.apple.ResourceFork`，不清除 quarantine 等安全属性，也不修改 SDK 或游戏目录。ZIP 不携带扩展属性；打包后检查 CRC、解压后严格签名、实际架构和可执行文件哈希。ARM64 交付为 `artifacts/sdl3/macos-arm64/StudySteady.zip`，结果见同目录的 `build.json`。本机 Documents 的 FileProvider 会给 `.app` 副本重新添加 FinderInfo，该副本严格签名验证失败已记录，因而应分发通过验证的 ZIP。这里只使用本地 ad-hoc 签名，没有 Apple Developer ID 签名或公证。

随后启动的 ARM64 完整编译发现了此前四个数学源语法检查未覆盖的 ABI 问题：SDK desktop `sgl_opengl_extension.h` 只在 x86_64 分支使用系统 `ptrdiff_t`，其他 CPU 则自行定义为 `int`。这会让 ARM64 的 `GLintptr/GLsizeiptr` 错为 32 位，并与 Loquaty 的未限定 `ptrdiff_t` 查找产生歧义。生成 overlay 已将两种 GL 类型精确改为 `std::ptrdiff_t`，移除错误的全局 typedef；随后完整编译和最终链接通过。ARM64 完整构建记录保存在 `build/macos-sdl3-arm64/cross-build.log`，最后增量及打包记录保存在 `build/macos-sdl3-arm64/cross-build-final.log`。

补充的全量语法扫描发现并已修复 SDK heap 和新版 JIT 在 `sysconf` 失败时引用未定义 `PAGE_SIZE` 的编译问题：正常路径仍查询实际页大小；heap 回退到 SDK 的 `sizeMinPage`，JIT 分配查询失败则返回空指针。`glscs_sakura2_jit_native_compiler.cpp` 仍包含 `mmap(PROT_READ|PROT_WRITE|PROT_EXEC)`；编译通过不证明 Apple 目标允许运行这段分配。当前游戏入口使用传统 Cotopha 解释器，应明确新版 JIT 是否需要、如何禁用或如何报告不可用，再决定是否实现额外的可执行内存后端。

### Windows：仍需单独完成基础层

以下是实际源级阻碍，并非仅缺少一个 CMake 平台开关：

- `native/compatibility/sdk/legacy/windows.h` 是本项目的有限兼容头，会遮蔽系统同名头；其中 `CRITICAL_SECTION` 仍是 `pthread_mutex_t`。
- `native/compatibility/sdk/legacy/platform.cpp` 仍使用 `pthread_*`、`localtime_r`、`dlopen/dlsym`，且包含 `<sys/mman.h>`。这些不是 MSVC 原生接口。传统存档原子替换还使用 `open/fcntl/fsync/rename/O_NOFOLLOW` 等 POSIX 语义。
- SDK `sakura_cpp_presets.h` 看到 `_WIN32` 后仍选择 `__PLATFORM_WINDOWS__`。只有 OpenGL overlay 将 WGL 分支换成专用宏，其他 common 源还可能选择 Win32 文件/线程/句柄接口。当前 SDL 窗口也没有实现旧 Windows 分支要求的 `GetWindowHandle()`。
- CMake 中有 GCC/Clang 专属编译参数，旧兼容原子实现使用 `__atomic_*` builtins。MSVC 不能直接套用这些选项和实现。
- 新的等待队列与逻辑线程 ID 使用 C++ 标准同步设施，不直接调用 pthread/futex syscall；但 futex word 的原子读取和生成的 SDK 原子交换仍使用 `__atomic_*`。因此也不能把这一小模块表述为已经过 MSVC 编译验证。
- LP64 与 LLP64 不同：SDK Unix 头的 `LONG/ULONG` 使用 C++ `long`，Windows 的 `long` 为 32 位；`wchar_t` 宽度也不同。不能为了消除编译报错把所有类型统一替换而不检查序列化和 FFI 边界。
- 原 SDK Windows CPU 分支只处理 `_M_X64/_M_AMD64/_M_IX86`，没有 Windows ARM64 的 `_M_ARM64` 分支；当前 Unix ARM 修复不会覆盖它。Windows ARM64 必须单独验证指针宽度与 SIMD，不能沿用 macOS ARM64 的通过结论。

可分块完成的路线是：先把 legacy 兼容层的锁、时间、静态符号解析改为独立命名的 SDL/C++ 接口，消除对同名系统 `windows.h` 的歧义；再拆 SDK 的 OS/renderer 选择，最后实现 Windows 的原子存档替换和 UTF-8 文件路径处理。保留已有固定宽度 wire-format 读写规则，并在 Windows 上重跑状态保存/恢复测试。MinGW 带有部分 POSIX 支持也不等于这些宏、句柄和类型问题已经解决。

### Linux

当前公共同步代码已避免 Linux futex 被错误地用于所有 Unix 系统，文件层仍大量使用 POSIX，Linux 因而更接近当前方向。实际 Linux 构建仍需检查：

- SDL 的真实窗口/音频驱动是否被编入；dummy 驱动不能证明桌面渲染工作。
- `find_package(OpenGL)` 的 GL/GLU 依赖与动态函数加载是否一致。
- 大小写敏感文件系统上的 SDK overlay 路径和游戏资源查找。
- x86_64/ARM64 分别构建，按实际 CPU 编入/屏蔽 SIMD 实现。

## 已执行的可重现检查

- `tests/integration/platform/CMakeLists.txt`：独立 PCM、图片测试。PCM 检查实际输出样本、位置、暂停/循环、回调退出，以及活动/未来播放器 master gain；图片检查 PNG alpha、JPEG 颜色与短写失败。
- `build/sdl-platform-audit/macos-sdk-syntax.json`：首批 17 个真实 macOS SDK/平台编译命令的 syntax-only 结果，17/17 通过。
- `build/sdl-platform-audit/macos-arm64-neon-syntax.json`：矩阵/碰撞及两个 NEON 源的 ARM64 语法检查，4/4 通过。
- `build/sdl-platform-audit/macos-sdk-remaining-syntax.json`：其余 274 个 SDK/平台 TU 的首扫记录，269 通过、5 失败；保留首扫失败现场。
- `build/sdl-platform-audit/macos-sdk-failure-recheck.json`：依据更新后生成副本和编译命令，5 项失败复查全部通过。修复包含页大小 fallback、SDL/非 WGL 初始化分支、录音明确不支持、应用 UUID/环境枚举。合并首批、补充扫描与修复复查，所选 291 个 SDK/平台 TU 的语法检查均已通过；这仍不是链接或游戏运行结论。
- `build/macos-sdl3-arm64/cross-build-final.log`：`python3 tools/build/build_sdl_desktop.py --arch arm64 --jobs 4` 最后增量构建和打包成功，包含 GL 版本/GLSL 能力修正。`otool` 确认最终 Mach-O 最低系统版本为 11.0；ZIP 解压后的签名及架构验证通过，未运行 ARM64 游戏。
- `build/macos-portability-audit-final/results.json`：使用实际编译命令检查86个 legacy/platform/application TU，全部通过。后续完整 macOS 构建和运行证据比这批语法检查更强，历史失败记录保留用于追踪修复。
- `build/sdl-macos-smoke/title-run.log`、`window-probe.log`：真实标题画面、SDL 输入和窗口绘制；截图位于 `artifacts/sdl3/macos-x86_64/`。`opening-probe.log` 记录原始 `opening.mei` 两轮数秒播放、不同帧实际绘制、停止线程、关闭及重新打开；日志中的旧 “Android GL window” 名称是跨平台 probe 遗留文本，该次实际宿主为 Intel macOS。
- `build/sdl-font-check/self-test.log`：独立链接现有完整 SDK/legacy 对象与最终字体适配源，读取原游戏资源、使用独立本地目录和 dummy 音频运行全部 legacy 自测，退出0；18项真实 glyph 检查及全部 legacy subsystem 通过。此记录不是 Android 真机或 ARM64 运行结果。
- `artifacts/studysteady-sdl3-arm64-dev.build.json`、`artifacts/sdl3/macos-x86_64/build.json`、`artifacts/sdl3/macos-arm64/build.json`：各交付物的架构、签名/归档校验和二进制摘要。重新打包后以相应报告的当前值为准；不能从 `runtime_verified_by_this_command=false` 推断其他单独运行记录不存在，也不能把它改写成 ARM64/Android 已运行。
- `python3 tools/ci/verify_sdl_inputs.py`：最终完整校验于2026-09-27 06:28 UTC通过，共6242项文件字节检查：EntisGLS 1805、SDL3 2183、Loquaty 446、TinyGLTF 1344、MotionPlayer/PSB 90、TJS及其依赖374。报告为 `artifacts/sdl3/input-integrity.json` 与 `.md`；范围不包括生成副本或游戏资源，不宣称导入的 Kirikiroid2 工作树本身是未修改的上游版本。

完整游戏验收应分别记录平台、架构、图形后端和实际设备。最低整合场景包括：标题与输入、背景与 E-mote、BGM/语音/MEI、存读档、前后台，以及关闭窗口后所有共享 GL 资源和后台音频线程的有序释放。
