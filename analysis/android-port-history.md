# StudySteady Android 移植

> 目录整理更新：文中历史路径 `EntisGLS/EntisGLS4.07.03/` 现已上移为 `EntisGLS/`；源码内容保持不变。`Primrose2` 与 `loquaty_lib_1.02` 发行目录已移除，构建仍使用 `vendor/official-loquaty` 源码。

目标设备：小米 10 Pro / Android 13 / arm64-v8a。

2026-09-27 起，SDK 构建输入改为 `EntisGLS/EntisGLS4.07.03/`，不再使用
根目录旧的 `EntisGLS4.07.03/` 或其中的预编译库。Android 引擎库从源码构建；
外部依赖来源与固定版本见 `vendor/official-entis-dependencies.json`。
下述 2026-09-23 真机结果属于迁移前 APK，不能直接视为新构建的真机验证。
本次完整 ARM64 编译、APK 签名、8 项绘制冻结主机测试和依赖来源检查通过；
手机未连接，尚未进行新 APK 的真机复测。详情见 `analysis/official-sdk-migration.md`。

**迁移前原生开发版已在真机推进至菜乃香第二十一章；用户当时已接受该状态并要求停止后续验证。完整通关仍未验证。**
传统 Cotopha `ECSContext` 已移植、链接进 ARM64 APK，并能读取原始 CSX 的
170 个类、2,858 个字符串和真实 `main(0x5460)` 入口。
基础运行时、窗口、输入和文字显示已通过真机测试。原始脚本的全局初始化、
全部对象/naked prologue 和真实 main 已执行，能显示原版语言选择页面并响应触摸，
继续加载 422 个资源、28 个界面。正式 APK 的 `EmoteDevice` 已初始化共享 GLES。
**已进入原版繁体中文标题菜单**：滚动背景、菜单和 `nak_a.psb` 角色均正常显示。
点击“从头开始”、确认姓名后，已进入原版开场剧情，背景视频、角色名和繁体中文正文
正常显示，触摸能推进台词。原版存档菜单已打开并实际写入含缩略图的存档；
已完成昵称语音后的保存、重启后从标题读档、继续对白和再次保存。剧情已推进到
菜乃香路线 nak_21，原生地图选择、历史回看/滚动/场景跳转、nak_07_1 与 nak_15 冷读档通过。
片头直接绘制的 Android 死锁已修复；真实窗口中完整 97 秒视频自然播放至末帧，
停止、关闭和重新打开通过。完整路线及后日谈未验证，已按用户要求停止继续测试。
双角色冷读档位置及持续动画已修复并真机通过；
新增颜色、波纹和模糊转场效果的像素及保存恢复自检通过，更多章节未验证。

## 已实现

- 项目内 Android NDK、JDK、SDK 安装与校验；不改系统工具链。
- EntisGLS4 的 Java/JNI/ARM64 运行库编译、APK 签名及真机安装。
- NOA 目录读取、原始条目提取、ERISAN 解压；游戏原文件保持原样。
- 从 EXE 的 `IDR_COTOMI` 资源提取启动配置，从 `script.noa` 提取主模块。
- 复制用户指定的 `motionplayer`、`psbfile`，保留许可证及逐文件 SHA-256。
- 从 `psbfile` 生成不依赖 TJS 的底层读取器，新增本游戏 PSB v4 头部解密、
  校验和检查、扩展资源表读取。
- 编译并验证 MotionPlayer 插值、真实 TJS/NCB/Player/EmoteEngine、控制器和 GLES 绘制。
  独立真机已正确绘制 `haz_a.psb`；正式脚本也已加载并显示标题角色 `nak_a.psb`。
- 16 个原始 NOA 包传入手机并逐个 SHA-256 核验；保留原配置的查找顺序，
  R18 补丁和 `voice.noa` 继续由游戏脚本动态挂载。
- 原始压缩 CSX 直接从 Android NOA opener 加载；传统文件适配层已在手机上
  验证内存文件、独立复制、超过 4 GiB 的定位和存档目录读写。
- 实际 `haz_a.psb` 的 4096×4096 RGBA 图集在 Adreno 650 上传、回读完全一致。
  这项验证只覆盖贴图，不代表角色合成和动画已完成。
- 传统核心对象、执行映像、文件、线程与当前 Android 环境的原生连接。
  真机已验证引用生命周期、普通赋值、64位算术、UTF-16内存和对象序列化、
  ERISAN压缩对象往返及NOA读取；线程执行/重启/暂停/恢复/终止及递归互斥也通过。
- 实际ERI解码、MIO静音播放/停止、Sprite图像创建、属性和父子关系通过。
- 显式延迟原生方法绑定，按真实接收对象解析方法；未知对象或方法仍明确失败。
- Window / InputFilter / ResourceManager / MessageSprite / 常用Setup接口移植。
  原始语言选择skin的5个资源、页面按钮、Android触摸到窗口命令链路已真机通过；
  日文/换行/ruby、逐字显示、Flush/Clear及原始input.xml映射均通过。
- 真实TJS/NCB以及MotionPlayer CPU对象层独立构建；真机可加载 `haz_a.psb`，
  创建26个节点和13个子Player，3个初始参数均为30。45个motion、379个node的
  2940次正反向求值通过；新版meshCombinator的78个节点、278个轴已接入真实节点。
  ARM64数学核3505组结果与原Windows DLL机器码逐位一致。真实 metadata 与 selector
  已接入，修复重复颜色交换和多组手臂同时显示；共享纹理跨线程像素逐字节一致。
- APK已统一使用真实TJS版PSB读取器及其TJSAlignedAlloc所有权；独立的旧无TJS
  probe仍可另行构建，但不与它一起链接到APK。
- 原版 `String.Calculate` 对象表达式解释器已移植，真机验证运算顺序、变量、
  成员条件、Unicode、赋值和错误路径；不依赖 x86 JIT。
- 鼠标/键盘/计时器/命中检测通过独立 callback ECSContext 执行，主栈保持独立。
  已验证原版按钮 XML 长按重复逻辑及回调所有权转移和关闭顺序。
- AudioPlayer 真实 PCM 语音片段替换、MIO 单次/循环切换、非线性音量曲线通过真机测试。
  MEI 视频解码、画面推进、停止和按帧定位通过真机测试。

当前全量资源检查：基础包 68 个 + R18 补丁 18 个，**86/86 个 PSB 通过**
头部校验、对象树遍历与扩展资源范围检查。86 个资产也全部通过真实 ARM64
Player 创建、基础姿态的两帧 GLES 绘制（共172帧）和纹理/实例释放检查；
这项批量检查不代表全部命名动画或完整游戏流程已验证。

PSB 容器版本 `4` 与内部 motion 数据版本 `3.03` 是不同的版本号。
当前头部解密适配仅接收本游戏已验证的 v4 / flags 0 或 1，其他加密模式会拒绝。

## 构建

本工程当前构建脚本针对本机 macOS Intel；目标为 Android ARM64。
需要保留本目录下的 `EntisGLS`、`StudySteadyR18` 和 `vendor`。
SDK 路径统一由 `cmake/entis_sdk.cmake` 与 `tools/entis_sdk.py` 指向新包；
传统运行时直接读取新包已展开的 `EntisGLS3/`，不再读取旧 `build/legacy` 缓存。

```sh
python3 tools/setup_android.py
python3 tools/build_android.py
```

产物：`artifacts/studysteady-arm64-dev.apk`。
这是本地开发签名，包名 `io.studysteady.port`，minSdk 29 / targetSdk 33。
当前引擎、Loquaty 与移植层统一以 Release 编译，避免 ESLObject 的调试布局
与发布布局混用。`cmake/official_entis.cmake` 按官方 Android 构建清单编译引擎；
Loquaty、TinyGLTF 使用 `vendor/official-*` 中固定版本的官方上游源码。
所有兼容修改保存在项目适配层或构建目录生成副本中，SDK 原文件保持不变。

构建采用 Unix Makefiles。可用 `STUDYSTEADY_CMAKE` 指定 CMake；本机备用路径
存放于忽略版本控制的 `.android-tools/build-host.json`，不会改动系统工具。

原始 Java 目录使用条件预处理，构建时生成到 `build/apk/java`。
未被游戏调用且无法编译的相机示例 `CameraCapture.java` 不纳入此运行库。

## 资源检查

```sh
python3 tools/noa.py StudySteadyR18/script.noa
python3 tools/noa.py StudySteadyR18/script.noa --extract script.csx --output build/game/script.csx
python3 tools/audit_game.py
```

检查结果写入 `artifacts/game-audit.json`，包括所有根目录条目的编码模式、
PSB 头部校验、对象树遍历和 v4 扩展资源引用检查。
这不验证最终角色画面、动画时序、剧情、音频或存档。

移植版的 PSB 读取代码由 `tools/prepare_psb_port.py` 从导入文件生成到
`build/generated/psb`。修改生成步骤或 `native/psb_*.h`，不要直接改生成结果。
源项目 `/Users/fenghengzhi/Developer/kirikiroid2-web` 没有被修改。

## 真机验证

```sh
.android-tools/platform-tools/platform-tools/adb install --no-incremental -r artifacts/studysteady-arm64-dev.apk
.android-tools/platform-tools/platform-tools/adb shell am start -n io.studysteady.port/.GameActivity
python3 tools/sync_game.py --serial <adb设备序列号>
```

可选的 PSB 测试文件放在应用自身外部目录的 `files/game/haz_a.psb`。
它仅用于验证原生解析器，不会显示角色。完整游戏资源由同步脚本复制到
应用自身外部目录 `files/game/`，未打包进 APK；同步后再次启动应用。
同步脚本不改动存档。首次同步约 10.8 GB，已存在且校验一致的包会跳过。
日志标签为 `StudySteady` / `EntisGLS`。

如果资源已放入正确目录，但启动日志对 `.noa` 报 `Permission denied`，
可用以下命令重建资源文件的访问权限。小米 14 Pro / Android 16 上曾出现
导入文件存在、大小正确，但游戏进程无法读取的情况；重新创建文件后可读取。

```sh
python3 tools/sync_game.py --serial <adb设备序列号> --repair-permissions --report artifacts/game-sync-repaired.json
```

执行前关闭游戏。修复模式先上传并校验临时文件，再将旧文件保留为同目录的
隐藏备份并替换正式文件；不改存档，也不修改电脑上的原始资源。
全量修复需额外约 10.8 GB 空间，备份不会自动删除。脚本校验的是 ADB shell
读取到的 SHA-256，完成后还需启动游戏确认应用本身能读取资源。

默认启动直接执行原始游戏。开发自检通过独立入口执行；每次独立测试前先
`adb shell am force-stop io.studysteady.port`，再使用下列启动参数：

```sh
.android-tools/platform-tools/platform-tools/adb shell am start -n io.studysteady.port/.GameActivity --es ARGUMENT --self-test
.android-tools/platform-tools/platform-tools/adb shell am start -n io.studysteady.port/.GameActivity --es ARGUMENT --emote-probe
.android-tools/platform-tools/platform-tools/adb shell am start -n io.studysteady.port/.GameActivity --es ARGUMENT --image-export-probe
.android-tools/platform-tools/platform-tools/adb shell am start -n io.studysteady.port/.GameActivity --es ARGUMENT --opening-probe
.android-tools/platform-tools/platform-tools/adb shell am start -n io.studysteady.port/.GameActivity --es ARGUMENT --opening-full-probe
```

`--self-test` 包含对象运行时、输入、音视频、触屏按钮和存档边界测试；
`--emote-probe` 检查真实角色销毁后读档与绘制；`--image-export-probe` 检查图片编码和缩略图。
`--opening-probe` 在真实窗口播放原始片头两轮，各 3 秒；`--opening-full-probe`
让首轮自然播放至结束（约 97 秒），检查末帧、耗时、实际 GL 帧及关闭后重开。


## 未验证范围与后续事项

1. 继续验证路线中后段选择、存读档、片尾、后日谈与完成后的持久解锁。
   此前读档引用、角色首帧和持续刷新问题已修复，并通过实际游戏冷读档。
2. 扩大实际角色时间线、语音口型与场景合成的覆盖；86 个资产的基础姿态检查
   不代表全部动画、条件分支或完整路线已经验证。
3. 首次资源部署仍使用 ADB 同步，手机端资源导入界面未实现。当前测试手机
   已完成约 10.8 GB 资源部署与校验；APK 本身不包含游戏资源。

取证和验证范围见 `analysis/native-port-status.md`。

这里的“传统 Cotopha”指对象模式运行时这一架构层，不是一个已核实的小版本号。
新包内同时包含 Android Sakura2 运行时与已展开的 `EntisGLS3/` 中的
`ECSContext` 源码；当前 APK 已在前者的Android底座上接入后者移植的对象执行层。
目前没有确认游戏内 `ECSContext` 与归档源码逐函数一致，也没有证据证明
游戏里的 `ECSContext` 版本更高。SDK 包的发布日期不能直接代表其中每个组件的版本。

新 SDK 的 Android 支持包含库源码、Java/JNI 和构建模板；需要额外适配的是
本游戏现成脚本使用的传统对象模式。Android 构建清单包含当前 glscs/Rosetta 等
实现，而 `glscs_context.cpp` 与 `glscs_execution_image.cpp` 列在单独的 Windows
GLS3 工程中。使用 Android 已支持的接口开发应用，不需要额外接入 ECSContext；
继续运行本游戏原始 `script.csx` 则需要相应对象模式兼容能力。

具体实现边界：`native/legacy_compat/README.md`、`native/motion_bridge/README.md`。
