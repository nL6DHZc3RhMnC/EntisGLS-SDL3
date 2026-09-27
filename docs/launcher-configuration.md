# EntisGLS 启动配置

启动器从所选游戏目录读取配置，当前执行的是**传统 Cotopha `ECSContext` 的 `.csx` 映像**。EntisGLS 名称覆盖多个时期的运行时；`.lqs` / Loquaty 应用以及需要额外 Windows DLL 的游戏目前不在此入口的支持范围。能够解析配置并不等于所有游戏的 Native 接口都已实现。

## 配置发现顺序

1. 使用明确指定的配置文件（`--config`；可指定目录内 XML 或原始 EXE）。
2. 游戏根目录的 `entis-launcher.xml`。
3. 游戏根目录的 `cotopha.xml`。
4. 游戏根目录内唯一包含 `RCDATA / IDR_COTOMI` 资源的 Windows EXE。

EXE 只作为数据文件读取，不运行 Windows 代码。读取器支持 PE32 / PE32+，并解码 `IDR_COTOMI` 的 ERISAN 压缩配置。如果多个 EXE 都带启动配置，会要求明确选择；不会猜测主程序，也不会仅凭目录里存在某个 `.noa` 就默认是 StudySteady。

对已经严格识别的游戏，可以由兼容配置模块提供启动模板。这是单独的兼容处理，不是通用目录的默认配置。StudySteady 的旧 NOA 导入安装使用这一方式；一般游戏仍需其原始配置或手工编写的 XML。

## 最小示例

把以下文件保存为所选游戏根目录的 `entis-launcher.xml`。示例文件名需要改为该游戏的实际名称。

```xml
<?xml version="1.0" encoding="utf-8"?>
<script src="main.csx">
    <archive path="patch.noa"/>
    <archive path="data.noa"/>
    <file path="."/>
    <display caption="我的 Cotopha 游戏" width="1280" height="720"
             depth="32" frequency="60"
             CooperationLevel="window" change_mode="false"/>
</script>
```

`src` 来自原始配置，可以是直接文件，也可以是 NOA 内的条目名；启动器不固定为 `script.csx`。顶层的 `<archive>` / `<file>` 顺序会原样保留，这决定补丁包与普通资源的搜索优先级。配置提到但不存在的包会产生警告并保留挂载尝试，因为原版配置经常列出可选补丁或 DLC。真正的入口或必需资源缺失仍会导致启动失败。

宿主文件和目录路径会转换成 `storage://game/...`，指向当前所选游戏目录。兼容原配置中的 `$(CURRENT)\...`、反斜线、`./` 及已有 `storage://game/...` 路径。入口和字体的普通相对路径仍通过引擎的文件/归档搜索系统解析。

绝对路径、`..`、其他 URI 协议、未知 `$(...)` 环境变量及解析到游戏目录外的符号链接会被拒绝。配置不支持下载器、更新器、外部程序命令或动态配置文件；不会据此启动外部程序。

## 存档与游戏标识

原配置中的 `<save_dir>` 固定替换为 `storage://game/savedata`，对应当前游戏目录下的 `savedata`（`$(CURRENT)\savedata`）。Android、macOS、iOS 和其他使用公共 SDL 启动流程的平台遵守同一规则。游戏目录需要可写，原配置的 `accept_other_dir` 不会沿用。

目录不存在时自动创建空目录，存在时直接沿用。不查找、不复制或迁移老版本应用内部存档。启动器设置和 PSB 缓存仍放在应用数据目录。

默认 `gameId` 由规范化后的配置、声明的资源包、入口脚本的文件大小及前 64 KiB 内容形成确定性标识。它不包含宿主绝对目录，因此移动完整游戏目录会保持标识；配置或这些资源的变化可能改变标识。该值用于启动器设置和缓存，不是资源完整性或防篡改证明；当前存档位置固定在游戏目录，不随 ID 变化。

如果需要跨补丁版本保持明确的启动器设置身份，可指定稳定的 `id`：

```xml
<script src="main.csx" id="my-cotopha-game">
    <archive path="data.noa"/>
    <file path="."/>
</script>
```

`id` 限 1–80 个 ASCII 字母、数字、`-`、`_`、`.`；不能是 `.`、`..`、以点结尾或 Windows 保留设备名。不同游戏不应使用相同显式 `id`，相同 ID 表示有意共享启动器设置和缓存；不同游戏目录的 `savedata` 仍各自独立。显式 ID 优先于已识别游戏的默认兼容标识。

## 字体

字体从用户游戏的资源中读取，不要求将商业游戏字库打包进通用启动器。

```xml
<script src="main.csx">
    <archive path="data.noa"/>
    <file path="."/>
    <fonts>
        <file name="DialogueBitmap" path="dialogue.bmf"/>
        <file name="@DialogueBitmap" path="dialogue-vertical.bmf"/>
        <file name="Dialogue" path="fonts/dialogue.otf"/>
        <filter in="Default" out="Dialogue"/>
        <filter in="@Default" out="@DialogueBitmap"/>
    </fonts>
</script>
```

- `.bmf` 交由 EntisGLS 位图字库加载器处理，`name` 必填，可设置 `cache_kb`。
- `.ttf`、`.otf`、`.ttc`、`.otc` 交由跨平台字体适配层处理；`name` 是可选的独立注册名，省略时使用字体实际 family。同一 family 的 Regular/Bold 文件可使用不同 `name`；集合字体目前读取首个 face。
- `<filter in="别名" out="已有字体"/>` 在字体加载后注册。别名之间有依赖时，启动器会先注册被依赖者；循环或重复的 `in` 会明确报错。
- `Default` 与 `@Default` 需要明确映射到游戏可用的字体。通用模式不会把所有游戏的默认字体都指向 StudySteady 的 `MsgFont`。

只有显式启用或严格识别的兼容模块，才会添加其已验证的额外字体规则。

## E-mote / PSB 与兼容配置

启动器没有按游戏内置的 PSB 解密参数。未加密 PSB 直接验证并加载；当前加密解码支持 PSB v4 的头部加密（flags=1）。正文加密等未支持模式会明确报错。

加密 PSB 的参数来源按以下顺序选择：

1. 启动参数 `--psb-key`（Android 游戏设置也通过这个入口传入）。
2. 该游戏在启动器数据目录中的手动设置 `psb-key.txt`。
3. 用户 XML 的 `psb_key` 属性。
4. 读取游戏根目录的原始 E-mote DLL，识别候选参数并以实际 PSB 校验。

手动值支持无符号 32 位十进制或 `0x` 十六进制，**0 是有效的显式值**。手动值验证失败时不会回退到 DLL，以免掩盖配置错误。XML 示例中的值仅表示语法，需替换为该游戏的实际参数：

```xml
<script src="main.csx" psb_key="0x12345678">
    <archive path="data.noa"/>
    <file path="."/>
</script>
```

`entis-launcher.xml` / `psb_key` 是本启动器新增的配置约定；原游戏不一定自带这个文件。自动发现不要求创建 XML，更不会把发现的参数写进原游戏目录。

自动发现只把 DLL 作为 PE 数据文件读取，不加载或执行其中的代码。目前识别 `.data` / `.rdata` 中与 E-mote cryptkey 标记相邻的十进制候选；再检查真实 PSB 的版本、Adler-32 与段偏移。不同 DLL 版本未必保留这个布局；无有效候选或有多个有效候选时要求用户提供匹配驱动或手动配置。

成功结果记录在应用数据目录的 `games/<game-id>/psb-key-cache/`。指纹覆盖根目录 DLL 集合及完整文件内容。每次仍检查当前 DLL 和实际 PSB；缓存被篡改、DLL 改变或缺失时，不会盲目使用旧值。缓存写入失败只报告警告，不阻止已经验证成功的加载。

macOS 从游戏选择后的 **PSB settings** 进入设置，也可用以下命令：

```sh
EntisGLSLauncher --game-dir /path/to/game --configure-game
EntisGLSLauncher --game-dir /path/to/game --save-psb-key "$PSB_KEY"
EntisGLSLauncher --game-dir /path/to/game --save-psb-key auto
```

Android 在游戏列表中选择游戏后设置参数，或用 **补充 E-mote 驱动文件** 补充原驱动。对于直接授权的目录，验证后的 DLL 以新的独立文件名写入所选目录，不覆盖已有 DLL；旧版应用内副本仍支持原有导入方式。旧版只导入 NOA 的安装不需要重导全部资源，但需要补充匹配 DLL 或手动提供参数。清空/恢复自动表示不再手动覆盖，仍可使用 XML 的设置。

参数、缓存和 DLL 内容变化不进入游戏存档 ID；更换参数不会改换存档目录。本次存档策略不包含老版本应用存档迁移。

`profile="study-steady-r18"` 仅保留字体等兼容行为，已不提供解密参数。未知 profile 仍会报错。

## 当前 XML 范围

支持 UTF-8 的 `<script>` 或 `<cotopha>` 根元素；元素属性必须带引号。允许 XML 声明、注释、五种标准实体和 Unicode 数字字符引用。拒绝 DTD、自定义实体、CDATA、任意文本节点及超限/截断配置。旧 EXE 配置里非法 UTF-8 字节会替换为 `U+FFFD` 并记录警告。

主要支持以下元素：

| 元素 | 保留内容 |
| --- | --- |
| `archive` | `path`、`id`、`key`、`default_dir`、`fragment`、`fragment_cache`、`encrypt32` |
| `file` | `path`、`id`、`fragment`、`fragment_cache`；不支持嵌套下载条目 |
| `fonts` | `file` 和 `filter`，如上文 |
| `display` | `caption`、尺寸、色深、频率、合作模式、切换模式；最多一个 |
| `sound` | 频率、声道、采样位数配置 |
| `vm` | 堆/栈及边界参数；当前运行时强制关闭 JIT |
| `opengl` | SDK 图形能力开关和骨骼/灯光/阴影数量限制；不接受外部 shader cache 路径 |
| `save_dir` | 固定替换为当前游戏目录的 `storage://game/savedata` |
| `icon` | 忽略 Windows 资源图标 |

其他元素或不受支持的属性会明确报错。`module` 中的 Windows DLL 不会被加载；需要对应的跨平台实现后才能支持依赖它的游戏。

配置测试：`launcher_config_test <临时目录> [原游戏目录]`。测试无需启动 GUI 或初始化 SDL，覆盖配置发现、真实压缩 EXE 资源、字体依赖顺序、搜索优先级、存档身份与错误路径。
