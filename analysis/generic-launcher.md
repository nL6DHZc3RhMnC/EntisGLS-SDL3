# 通用 EntisGLS 启动器改造（0.3.0 / build 4）

本文记录 0.3.0 的改造过程。0.3.1 已移除下述历史版本的固定 PSB 参数，当前行为见 [PSB 参数发现](psb-key-discovery.md)。

本次把 SDL 启动路径从固定游戏改为配置驱动。开发入口、应用名称和安装包为 EntisGLS Launcher；当前的解释器仍是传统 Cotopha ECSContext / 对象 CSX，不代表已实现全部 EntisGLS 版本和所有游戏插件。

## 主要变更

- `native/launcher/game_config.*`：无需初始化 SDL/SDK 的 XML / PE32 / PE32+ 配置发现；从 IDR_COTOMI 提取并解码 ERISAN 配置，不执行 EXE。明确配置优先，含多个候选时报错。保留有序资源搜索、窗口参数、字体定义；动态读取入口名，不固定 script.csx。
- `native/legacy_runner.cpp`：使用已规范化的内存配置加载环境，按配置打开任意 CSX；字体通过环境的资源搜索器读取，支持 NOA 中的字库；失败原因返回启动界面。
- `native/platform/sdl/opentype_font.*`：独立注册显式名称与真实 family，同 family 的不同 face 可分别命名，避免 Regular/Bold 被合并。未声明的游戏字体不会全局注入。
- `native/launcher/compatibility_profiles.*`、`known_game.*` 与 `assets/compatibility/`：StudySteady 的 PSB key、字体别名、可选 Noto 和 NOA-only 模板隔离为独立 profile。自动识别要求 script.noa 的完整大小与 MD5 匹配，不依赖目录名或同名文件。显式 PSB key 优先。
- `native/sdl_main.cpp`：桌面游戏目录列表、明确配置选择、按游戏 ID 隔离存档。已知旧游戏的旧存档按需复制，保留原始数据且不覆盖已有新存档。用户显式 ID 优先；inspect 不创建游戏存档或迁移数据。
- `android/sdl/`：多游戏导入列表，递归复制完整选定目录并保留子目录，不再固定 16 个 NOA；各导入拥有独立 UUID 目录和收据。后台校验、取消及事务恢复；旧 game 目录作为历史条目保留。
- 构建默认使用 `ENTISGLS_LAUNCHER=ON`，内部旧目标和宏保留兼容。独立验证 FreeType，不依赖商业游戏目录；Noto 为可选 OFL 兼容资源。Android 保留旧包 ID 与签名以支持升级，新的安装包文件名不覆盖旧版。

配置示例与格式限制见 [配置说明](../docs/launcher-configuration.md)。

## 验证

已完成：

- 配置模块 53 项测试，无 SDL/SDK 初始化：真实 EXE 压缩资源、PE32+、不同入口、搜索顺序、身份分离/搬移稳定、字体依赖、错误路径。
- 字体/profile 单测：无游戏资源的通用模式、真实 Arial 与 CJK 栅格、Regular/Bold 独立 face、字号/样式/缓冲区/并发/生命周期、别名优先级、PSB key 隔离。
- 原创 523 字节 `adventure.csx`：从真实 CSX 序列化生成，经完整启动器 LoadEnvironment / ReadExecution / InitializeContext / main 成功执行；无 StudySteady 资源和字体。测试包含目录移动、显式 ID 隔离、缺失入口、拒绝错误 VM、拒绝通用游戏调用特定自测。
- 旧存档复制/保留新版存档、已知游戏尊重显式 ID。
- Android Java/DEX/Manifest 编译；18 项生产导入代码的模拟 provider/路径测试；带可选字体和不带字体的打包检查。
- Android ARM64 原生库与 APK 构建、相同开发签名、ELF/ZIP 16 KB 对齐、CRC、包内库/字体/兼容模板哈希；不含 NOA/CSX/EXE 或根目录 cotopha.xml。未安装或在手机上测试此版本。
- macOS x86_64 / arm64 Release 编译；两个 ZIP 解压后的架构和严格 ad-hoc 签名验证。Intel 最终 ZIP 的自测、繁中姓名/昵称、前三段正文与截图复查通过；ARM64 未在真机执行。
- 原始 SDK、SDL 和官方依赖输入完整性：6242 次文件校验通过。旧安装包保留。

机器可读报告：

- `artifacts/entisgls-launcher/generic-tests/report.json`：Debug 全启动验证。
- `artifacts/entisgls-launcher/generic-release-tests/report.json`：最终 Intel ZIP 的通用全启动验证（含 Unicode 入口）。
- `artifacts/entisgls-launcher/study-steady-regression/runtime-verification.json`：最终 Intel ZIP 的 StudySteady 繁中回归。
- `artifacts/entisgls-launcher/input-integrity.json`：上游输入复查。
- `artifacts/entisgls-launcher/macos-{x86_64,arm64}/build.json`：签名与包验证。
- `artifacts/entisgls-launcher-arm64-dev.apk.{build,validation}.json`：Android 包验证。

可复现测试命令：

```sh
cmake --build build/macos-sdl3 --target launcher_config_test launcher_fonts_test make_csx_fixture
build/macos-sdl3/launcher_config_test build/config-tests StudySteadyR18
python3 tools/verify_generic_launcher.py \
  --binary build/macos-sdl3/EntisGLSLauncher.app/Contents/MacOS/EntisGLSLauncher \
  --fixture-generator build/macos-sdl3/make_csx_fixture \
  --known-game-dir StudySteadyR18
python3 tools/verify_sdl_zhtw_macos.py \
  --output-dir artifacts/entisgls-launcher/study-steady-regression
python3 android/sdl/tests/test_import_discovery.py
```

## 尚未覆盖

未提供第二款商业游戏，因此跨游戏的实际可运行范围仍需样本验证。通用启动和配置机制已用独立原创 CSX 验证，但无法替代对其他游戏 Native/插件调用的回归。Sakura2/Loquaty、Windows DLL 模块及未移植接口明确不受支持。默认 ID 会随配置/资源指纹变化，可用显式稳定 id 保持跨补丁存档身份。

iOS、Linux、Windows 的接口沿用 SDL 公共平台层，尚未完成这些目标的完整构建与运行。Android 新游戏库需真机验证导入、启动、返回列表和生命周期；本次不把主机单测或交叉编译写成手机/ARM Mac 实测。

最终 Intel 对话测试首次未采集到截图（退出码为 0、正文栅格日志正常）；仅补跑失败的对话用例后截图与日志均通过，无应用代码修改。姓名及正文截图均已人工视觉复查。新的 CMake 开关还通过了无旧 SDL 缓存选项的全新配置检查。
