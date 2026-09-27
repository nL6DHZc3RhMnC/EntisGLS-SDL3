# Windows 原版 StudySteady PSB 密钥来源

结论：这份游戏的固定参数 `851083516` 嵌入在原版 `emotedriver.dll` 中，以 ASCII 字符串保存。DLL 创建 E-mote 播放器时自行转换为整数，初始化 `StructCryptFilter`；游戏 CSX 的正常加载接口不传入此参数。

## 原始文件

- `StudySteadyR18/emotedriver.dll` SHA-256：`40d2e510106fc8a79e0fd07e5c0d4839d75503089d6fc9de0f0904f0ab73bd4d`
- `StudySteadyR18/ststeady.exe` SHA-256：`9750f67ae51ba158cda4f756ccbd0875fb837ddd15e84db713be53f8573d91b3`
- IDA 只分析 `build/analysis/` 下副本，分析副本与原文件哈希相同。

## DLL 中的直接证据

字符串 `851083516` 位于 DLL 文件偏移 `0x8A568`，`.rdata` 段，静态 VA `0x1008B768`（image base `0x10000000`）。其后紧接 `#c#r#y#p#t#k#e#y#` 标记；仅凭该标记不能判断原开发者具体采用何种构建/打包写入流程。

三处实际代码引用此字符串：

| 路径 | 地址 | 行为 |
| --- | --- | --- |
| `PEmotePlayer` 构造路径 `sub_10001F20` | `0x10002030` / `0x10002035` | 取固定字符串，转换成整数，构造解码状态后传给 PSB 对象 |
| `EmoteCheckValidObject` | `0x10003F0D` / `0x10003F33` | 使用同一字符串初始化解码器并检查 PSB |
| `EmoteFilterTexture` | `0x10003959` / `0x10003982` | 使用同一字符串初始化解码器 |

转换函数 `0x1006A546` 调用 MSVC CRT 的 `parse_integer<unsigned long,...>`，基数是 10。播放器构造时解码状态的四个值依次为 `123456789`、`362436069`、`521288629` 和解析出的 `851083516`。

简化等价逻辑如下，属于反编译结果的说明，不是取得了原厂源码：

```cpp
seed = parse_decimal("851083516");
filter = StructCryptFilter(123456789, 362436069, 521288629, seed);
playerPsb = PSBObject(psbBytes, psbSize, &filter);
```

实际加载路径为 `sub_10001F20 -> sub_10058450 -> sub_10058870 -> sub_100018E0`。`sub_100018E0` 生成 xorshift 字节流并 XOR；v4 且 flags bit 0 有效时处理 `[8,44)` 和 `[44,56)`。验证路径 `EmoteCheckValidObject -> sub_10058680` 还会校验解码后的 Adler-32。支持正文 bit 1 的代码也在 DLL 内，但本游戏此前检查的 PSB 为 flags=1，不能描述为整份 PSB 都加密。

## EXE 与脚本的职责

`ststeady.exe` 通过 PE 导入表直接依赖 `emotedriver.dll`，导入 `EmoteCreate(IEmoteDevice::InitParam const&)`，IAT VA 为 `0x8CB78C`。EXE 的 `sub_82DB20` 在 `0x82DBEE` 调用它；正常初始化传窗口/图形设备相关参数，DLL 内的解码常量不来自此调用。

已解压的原始 `script.csx` 中，`EmoteDevice.Initialize(Window)` 的调用位于 CSX image 偏移 `0x6561`；`EmoteSprite.LoadPlayer(EmoteDevice, String)` 的两处调用位于 `0x264CA`、`0x35247`。主路径只传设备和 PSB 资源路径，没有密钥参数。脚本证据见 `build/analysis/psb-key-origin-script.txt`。

因此运行时关系为：游戏脚本发出加载请求 → EXE 中的原生适配层提供 PSB 数据 → `emotedriver.dll` 使用自己内嵌的参数解码并创建播放器。此前移植代码先从 PSB 头提出候选、用实际 Adler-32 验证；本次补充的 DLL 引用证据直接证实原版也使用同一个值。

这里确认的是发行文件的运行时行为；无法仅从现有二进制区分该字符串最初由源码常量、链接过程还是 SDK 打包工具写入 DLL。当前跨平台兼容配置保存此值，是为了复现该 DLL 的解码行为，而不执行 Windows DLL。

原始反编译及交叉引用证据见 [psb-key-origin.json](psb-key-origin.json)。本次未修改游戏、SDK 或启动器实现。
