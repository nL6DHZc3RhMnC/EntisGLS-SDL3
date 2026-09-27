# EntisGLS Launcher

基于 SDL3 的通用 EntisGLS 启动器，当前接入的是传统 Cotopha `ECSContext` / 对象 CSX 运行时。
优先维护 Android ARM64 与 macOS；公共平台接口兼顾 iOS、Linux、Windows。
StudySteady 是兼容性样例，所需补丁独立放在 `native/compatibility/games/compatibility_profiles.cpp` 与
`assets/compatibility/`，普通游戏不会继承它的脚本入口、资源包清单、字体或 PSB 解密参数。

SDK 输入为 `EntisGLS/`，其中 `Cotopha/` 和 `EntisGLS3/` 提供引擎源码；适配在项目代码或构建时生成的副本中完成。
**构建不需要 `StudySteadyR18/`，安装包不包含商业游戏脚本或资源包。**
构建过程不修改原 SDK、游戏目录；已单独获取并附许可的 Noto 字体是可选兼容资源。

## 使用

开发版 **0.3.1（构建号 5）**：

- Android：`artifacts/entisgls-launcher-arm64-dev.apk`。从系统目录选择器添加游戏目录，
  持久保留该目录的读写授权，直接访问原目录，不复制整份资源。支持多个游戏目录。
  包名为 `io.entisgls.launcher`，沿用现有开发签名。
  它与旧包名 `io.studysteady.port` 属于两个独立应用，可以共存；旧应用的数据仍在旧应用中，
  不会自动迁移。新应用需重新选择游戏目录，旧包名应用中的存档需另行导出、迁移。
- macOS：`artifacts/entisgls-launcher/macos-x86_64/EntisGLSLauncher.zip` 或
  `artifacts/entisgls-launcher/macos-arm64/EntisGLSLauncher.zip`。
  解压打开 `EntisGLSLauncher.app`，在游戏列表中选择或添加目录。直接读取资源，不复制游戏。
  ZIP 使用本地 ad-hoc 签名，未进行 Developer ID 公证；最低 macOS 11.0。

识别顺序为显式配置、`entis-launcher.xml`、`cotopha.xml`、原游戏 EXE 的 `IDR_COTOMI` 资源。
EXE 只用于提取配置，不执行其中的 Windows 代码。多个候选不自动猜测。
只有命中已知完整脚本包指纹时，旧版 StudySteady 的 NOA-only 导入可使用独立启动模板；
加密 E-mote 资源仍需补充原版驱动 DLL 或在游戏设置中提供参数。

配置字段、路径、字体和存档规则见 [启动配置说明](docs/launcher-configuration.md)。
PSB 解密参数不按游戏内置：从用户设置、XML 或原版 E-mote DLL 获取，并使用实际 PSB 验证。
Android 可在游戏列表补充驱动文件；Mac 提供 PSB settings。自动缓存与手动设置均在应用数据目录，
不会写入原游戏文件。详情见 [PSB 参数发现与验证](docs/research/psb-key-discovery.md)。

存档统一使用当前游戏目录中的 `savedata`，即 `$(CURRENT)\savedata`，目录不存在时自动创建。
游戏目录必须可写；Android 通过所选目录的授权完成读写。已有 `savedata` 直接沿用，
不查找或迁移老版本应用内部的存档。
PSB 参数、缓存等启动器设置仍在应用数据目录，游戏 ID 不会因这次存档路径调整而改变。

## 兼容范围

这不是对全部 EntisGLS 游戏兼容的承诺。已接入的传统 CSX、NOA、ERI/MEI/MIO/BMF、
PSB/TJS/E-mote 路径仍受具体脚本版本、原生类和插件覆盖范围限制。
Sakura2/Loquaty 脚本入口、Windows DLL 插件及其他未适配接口不能直接运行。
配置不受支持时会报告原因；请提供其他游戏样本逐一验证。

macOS Intel 可执行本地运行验证；Apple Silicon 当前做交叉构建和包验证。
Android 的直接目录访问仍需真机复测。iOS 已通过设备包编译和包结构检查；游戏运行仍待验证。
Linux、Windows 尚未完成平台构建验证。
SDL 负责窗口、输入、音频、路径与同步；渲染继续使用 SDK 的 OpenGL/GLES。

## 项目布局

- `EntisGLS/`、`vendor/`：原始 SDK 与第三方输入。
- `native/`：共享启动逻辑、`runtime/cotopha_port/` 移植代码、文件接口、平台适配及 E-mote 扩展。
- `apps/`：公共入口和 Android、iOS、macOS 应用外壳。
- `tests/`、`tools/`：按用途组织测试、诊断、构建及 SDK 生成工具。
- `docs/architecture/`、`docs/development/`、`docs/research/`：架构、开发说明与历史研究。

目录职责和依赖边界见 [源码布局](docs/architecture/source-layout.md)。
传统运行时诊断探针默认不编入发布应用；需要时启用 `ENTISGLS_BUILD_DIAGNOSTICS=ON`。

## 构建

GitHub Actions 的 `Build and release` 统一构建 Android ARM64、macOS Intel、Apple Silicon 和 iOS ARM64 未签名 IPA。
推送 `main` 后全部构建和检查通过，会自动发布开发版 GitHub Release；
推送 `v*` 标签会发布对应版本。下载文件包含 APK、两种架构的 Mac ZIP、未签名 IPA、校验值和测试报告。
首次运行需配置持久的 Android 签名 Secrets；配置与触发规则见
[GitHub Actions 说明](docs/development/github-actions.md)。构建不需要提供游戏资源。

Actions 跨运行保留编译缓存和依赖下载缓存。编译缓存按平台、架构、Clang/Xcode/SDK 区分，
每次仍执行依赖校验、测试与打包；命中统计随 Actions 诊断报告提供。

iOS 为实验支持，设备包与其他平台一起构建和发布；当前工作流验证编译、未签名包结构，以及模拟器游戏库启动和 GLES 显示。
安装到真机前需要在本地使用自己的 Apple 账号签名。
构建、文件导入和签名限制见 [iOS 说明](docs/development/ios-build.md)。

```sh
python3 tools/sdk/setup_android.py
python3 tools/sdk/setup_sdl3.py
python3 tools/sdk/setup_sdl_fonts.py --freetype-only
python3 tools/build/build_sdl_android.py
python3 tools/build/build_sdl_desktop.py --arch x86_64
python3 tools/build/build_sdl_desktop.py --arch arm64
```

固定依赖包括 SDL3 3.4.16 和 FreeType 2.14.3。默认 CMake 构建通用启动器：

```sh
cmake -S . -B build/launcher -DENTISGLS_LAUNCHER=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build/launcher --target studysteady_sdl --parallel 6
```

内部目标名和 `STUDYSTEADY_SDL3` 宏暂保留用于旧工具链兼容；应用名称、配置与启动行为已通用化。
工具读取 `.android-tools/build-host.json`，可用 `ENTISGLS_CMAKE` 覆盖 CMake 路径。
`--freetype-only` 不读取游戏目录或要求兼容字库。Android 打包可用 `--without-bundled-fonts`。
历史 JNI 构建 `tools/build/build_android.py` 显式关闭通用启动器，旧 APK 和历史记录保留。

复查输入可运行 `python3 tools/ci/verify_sdl_inputs.py` 与
`python3 tools/sdk/setup_sdl_fonts.py --verify --freetype-only`。
测试与产物记录见 [通用启动器改造](docs/research/generic-launcher.md)，
早期过程见 [SDL3 迁移](docs/research/sdl3-migration.md)、[繁中修复](docs/research/sdl3-zhtw-fix.md)。
