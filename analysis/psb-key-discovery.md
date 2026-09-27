# PSB 参数发现：0.3.1 / build 5

本次已从启动器兼容配置、旧 JNI 路径和测试工具中移除真实游戏解密参数的固定值。字体等已知游戏兼容配置继续独立存在，但不再包含 PSB 参数。

## 当前流程

1. 未加密的 PSB 直接验证校验和及偏移，无需 DLL 或参数。
2. 加密 PSB 优先使用本次命令行/Android 游戏设置，其次是本机该游戏的设置文件，再其次是 XML 的 `psb_key`。
3. 没有显式值时，只读游戏根目录 DLL 的 PE 数据区，根据 E-mote 标记获取候选值。原 Windows DLL 不会被执行或打包进启动器。
4. 用当前 PSB 的 Adler-32、版本和段偏移验证候选；无有效值、多个有效值或显式值错误均返回清楚的错误，不静默回退。
5. 成功结果写入游戏的应用数据目录。DLL 集合与完整内容的 SHA-256 形成缓存指纹；每次仍检查实际 DLL 和 PSB，缓存不可写不阻止已经验证的加载。

核心实现：`native/launcher/psb_key_resolver.*`；设置：`psb_key_settings.*`；调用边界：`psb_key_runtime.*` 与 `legacy_emote.cpp`；Motion 原始读取回调：`native/motion_bridge/tjs_runtime/psb_storage.cpp`。当前自动提取支持带已知标记布局的 PE32/PE32+，解码范围仍为已验证的 PSB v4 flags 0/1；其他布局可以手动指定值，其他加密格式需要额外适配。

macOS 游戏选择后可进入 PSB settings，也可使用 `--configure-game` / `--save-psb-key`。Android 提供每游戏设置与“补充 E-mote 驱动文件”，后者只向应用自己的导入副本复制 DLL，不修改原来源，旧驱动备份为 `.bin`。旧 NOA-only 安装需要补充匹配驱动或提供参数，不再依赖启动器内置的游戏密钥。

参数、缓存和 DLL 内容均不参与存档 ID。对于旧 0.3.0 带 XML 参数的自动 ID，保留旧 ID 计算结果以尝试复制旧存档；源存档保留，已有新存档不覆盖。不同参数更换不会再改变新的自动 ID。

## 验证结果

- 独立 resolver 合成 PE32/PE32+、多候选、显式零/错误值、头部损坏、DLL/缓存篡改、完整内容指纹、缺驱动、缓存不可写等测试通过；ASan/UBSan 通过。
- 用用户原 DLL 与实际 PSB 验证动态发现通过，测试没有写入或打印真实参数。
- 设置模块 28 项、配置模块 59 项通过；后者包含 XML 参数变化/移除不改变 ID，以及原 ID 迁移标识。
- Motion 原始字节回调集成测试通过：优先级、不回退、未加密跳过参数获取且仍检查数据、线程及销毁生命周期。
- Android 生产设置/驱动复制的 JVM 模拟环境测试 32 项、原导入测试 18 项，以及 Java/DEX/Manifest 构建通过。
- 最终 Intel ZIP 的真实 NOA/Motion 流程 11 项通过：缺驱动错误、自动发现、缓存、驱动变更、保存/清除设置、命令行优先、无 DLL 手动加载、设置覆盖 XML、缓存不替代缺失驱动；所有情况存档 ID 相同。
- 最终 Intel / ARM Mac 可执行文件及 APK 的三个原生库均扫描不到原游戏参数的 ASCII 或 uint32 小端字面量；当前 native/tools/android/cmake 的执行源码也无该值。
- 原始 SDK 与依赖输入完整性 6242 次文件检查通过。原 SDK 和游戏文件不修改。
- Android 0.3.1-dev / code 5 APK 完整构建、相同开发签名、ZIP CRC、ELF 与 ZIP 16KB 对齐均通过；本轮未安装或真机测试。
- macOS Intel / ARM64 Release ZIP 解压后的架构和严格签名通过。Intel 最终包的运行时自测、繁中姓名/昵称、前三段正文与截图复查通过；ARM64 未实机运行。

参数来源与校验测试不需要运行 Windows 程序。测试用的覆盖值只在执行时从用户原始驱动读取，未作为预设编入源码或产物。

## 证据与复现

- `artifacts/entisgls-launcher/psb-discovery-runtime/report.json`：最终 Intel 包的 11 项参数流程。
- `artifacts/entisgls-launcher/psb-game-regression/runtime-verification.json`：最终 Intel 包繁中姓名/正文回归。
- `artifacts/entisgls-launcher/psb-no-embedded-key.json`：源代码与二进制字面量扫描。
- `artifacts/entisgls-launcher-arm64-dev.verification.json`：Android 包、签名、ABI、16KB 和旧 JNI 语法验证。
- `build/psb-key-{resolver,settings,config}-test.log`、`build/psb-key-callback-test.log`：原生单测。

```sh
cmake --build build/macos-sdl3-x86_64 --target \
  psb_key_resolver_test psb_key_settings_test launcher_config_test motion_psb_key_callback_test
build/macos-sdl3-x86_64/psb_key_resolver_test StudySteadyR18 build/game/haz_a.psb
build/macos-sdl3-x86_64/psb_key_settings_test
python3 tools/verify_psb_discovery_macos.py \
  --output-dir artifacts/entisgls-launcher/psb-discovery-runtime
python3 tools/verify_sdl_zhtw_macos.py \
  --output-dir artifacts/entisgls-launcher/psb-game-regression
python3 android/sdl/tests/test_psb_settings.py
```

首次参数回归使用了更宽的 Emote 存档/动画探针：自动发现、首次真实像素渲染通过，但后一次在“动画队列还未推进”的实时断言失败，日志中该次 PSB 缓存校验与加载均成功。保留原记录于 `artifacts/entisgls-launcher/psb-discovery/`；参数回归随后改用现有运行时自测入口，避免把实时动画断言当成参数检测结果，11 项全部通过。此处没有修改或放宽原动画探针断言。
