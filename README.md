# EntisGLS Launcher

基于 SDL3 的通用 EntisGLS 启动器，当前接入的是传统 Cotopha `ECSContext` / 对象 CSX 运行时。
优先维护 Android ARM64 与 macOS；公共平台接口兼顾 iOS、Linux、Windows。
StudySteady 是兼容性样例，所需补丁独立放在 `native/launcher/compatibility_profiles.cpp` 与
`assets/compatibility/`，普通游戏不会继承它的脚本入口、资源包清单、字体或 PSB 解密参数。

SDK 输入为 `EntisGLS/EntisGLS4.07.03/`，适配在项目代码或构建时生成的副本中完成。
**构建不需要 `StudySteadyR18/`，安装包不包含商业游戏脚本或资源包。**
原 SDK、游戏目录均不修改；已单独获取并附许可的 Noto 字体是可选兼容资源。

## 使用

开发版 **0.3.1（构建号 5）**：

- Android：`artifacts/entisgls-launcher-arm64-dev.apk`。从系统目录选择器导入游戏目录，
  在列表选择游戏。支持多个独立资源目录，复制资源及子目录，按游戏保存进度。
  包名为 `io.entisgls.launcher`，沿用现有开发签名。
  它与旧包名 `io.studysteady.port` 属于两个独立应用，可以共存；旧应用的数据仍在旧应用中，
  不会自动迁移。新应用需重新导入游戏，旧存档需另行导出、迁移。
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
不会写入原游戏文件。详情见 [PSB 参数发现与验证](analysis/psb-key-discovery.md)。

存档使用应用数据目录中的 `games/<game-id>/savedata`；普通游戏身份从配置和资源指纹生成，
可用配置中的 `id` 固定。更新游戏资源可能改变自动身份，长期使用建议显式指定稳定的 `id`。
迁移旧 StudySteady 数据时复制旧存档，不删除原文件、不覆盖新存档。

## 兼容范围

这不是对全部 EntisGLS 游戏兼容的承诺。已接入的传统 CSX、NOA、ERI/MEI/MIO/BMF、
PSB/TJS/E-mote 路径仍受具体脚本版本、原生类和插件覆盖范围限制。
Sakura2/Loquaty 脚本入口、Windows DLL 插件及其他未适配接口不能直接运行。
配置不受支持时会报告原因；请提供其他游戏样本逐一验证。

macOS Intel 可执行本地运行验证；Apple Silicon 当前做交叉构建和包验证。
Android 的新多游戏导入流程需要真机复测。iOS、Linux、Windows 尚未完成平台构建验证。
SDL 负责窗口、输入、音频、路径与同步；渲染继续使用 SDK 的 OpenGL/GLES。

## 构建

GitHub Actions 自动构建 Android ARM64、macOS Intel 和 Apple Silicon。
推送 `main` 后全部构建和检查通过，会自动发布开发版 GitHub Release；
推送 `v*` 标签会发布对应版本。下载文件包含 APK、两种架构的 Mac ZIP、校验值和测试报告。
首次运行需配置持久的 Android 签名 Secrets；配置与触发规则见
[GitHub Actions 说明](docs/github-actions.md)。构建不需要提供游戏资源。

iOS 使用独立的实验构建流程，生成 ARM64 未签名 IPA，并在 iPhone 模拟器上检查启动器界面。
通过后发布 `ios-dev-*` 预发布版本；安装到真机前需要在本地使用自己的 Apple 账号签名。
构建、文件导入和签名限制见 [iOS 说明](docs/ios-build.md)。

```sh
python3 tools/setup_android.py
python3 tools/setup_sdl3.py
python3 tools/setup_sdl_fonts.py --freetype-only
python3 tools/build_sdl_android.py
python3 tools/build_sdl_desktop.py --arch x86_64
python3 tools/build_sdl_desktop.py --arch arm64
```

固定依赖包括 SDL3 3.4.16 和 FreeType 2.14.3。默认 CMake 构建通用启动器：

```sh
cmake -S . -B build/launcher -DENTISGLS_LAUNCHER=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build/launcher --target studysteady_sdl --parallel 6
```

内部目标名和 `STUDYSTEADY_SDL3` 宏暂保留用于旧工具链兼容；应用名称、配置与启动行为已通用化。
工具读取 `.android-tools/build-host.json`，可用 `ENTISGLS_CMAKE` 覆盖 CMake 路径。
`--freetype-only` 不读取游戏目录或要求兼容字库。Android 打包可用 `--without-bundled-fonts`。
历史 JNI 构建 `tools/build_android.py` 显式关闭通用启动器，旧 APK 和历史记录保留。

复查输入可运行 `python3 tools/verify_sdl_inputs.py` 与
`python3 tools/setup_sdl_fonts.py --verify --freetype-only`。
测试与产物记录见 [通用启动器改造](analysis/generic-launcher.md)，
早期过程见 [SDL3 迁移](analysis/sdl3-migration.md)、[繁中修复](analysis/sdl3-zhtw-fix.md)。
