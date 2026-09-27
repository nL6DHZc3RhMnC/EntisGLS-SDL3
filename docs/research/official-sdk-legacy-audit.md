# 官方 SDK 的传统运行时来源审计

> 目录整理更新：文中历史路径 `EntisGLS/EntisGLS4.07.03/` 现已上移为 `EntisGLS/`；源码内容保持不变。`Primrose2` 与 `loquaty_lib_1.02` 发行目录已移除，构建仍使用 `vendor/official-loquaty` 源码。

日期：2026-09-27。仅比较本机用户提供的两份目录；本审计不独立证明下载来源或官方签名。

## 结果

新目录 `EntisGLS/EntisGLS4.07.03/EntisGLS3/` 已经包含展开后的 `ESL`、`GLS3`、`erisalib`、`EGL` 四个组件；没有 `EntisGLS3.7z`，无需从旧目录借用压缩包。

对旧 `EntisGLS4.07.03/EntisGLS3/EntisGLS3.7z` 在独立临时目录重新解压，按相对路径及完整文件 SHA-256 与新目录比较：

| 比较 | 文件数 | 内容不同 | 新增或缺失 |
|---|---:|---:|---:|
| 新目录 vs 旧压缩包新鲜解压 | 199 / 199 | 0 | 0 |
| 既有 `build/legacy` vs 旧压缩包新鲜解压 | 199 / 199 | 0 | 0 |

旧压缩包 SHA-256：`8cbbde1f8ae35af11f1814607ef97387c15400f098552e5f9c835040f8dde3b4`。

199 个文件（包括 159 个 `.cpp` / `.h`）的相对路径排序清单，以每行 `sha256 + 两个空格 + 相对路径 + LF` 编码为 UTF-8 后的 SHA-256：`d3e2b16f2111ef1952e68ed7ccf52a10f677f0cff4a526387dd992732c605eb2`。文件路径按 Python `str` 排序。

这只能证明传统 GLS3 子树相同，不能推出两份 SDK 的 Cotopha、Android 实现或预编译库相同。

## 生成适配的复核

直接以新展开目录为输入，在独立空临时目录执行：

```sh
python3 tools/sdk/prepare_cotopha.py \
  --legacy EntisGLS/EntisGLS4.07.03/EntisGLS3 \
  --output <empty-temporary-directory>
```

生成成功。164 个生成文件与审计时 `build/android-arm64/legacy_generated` 的 164 个文件全部逐文件 SHA-256 一致，没有新增或缺失。生成脚本包含已有的字符编码、Clang、ARM64、对象状态和序列化适配；它们仍能直接作用于新输入。

例如：

| 新官方包内文件 | SHA-256 |
|---|---|
| `GLS3/Source/glscs_context.cpp` | `ccdecf8dee1e40d6c8fe51a9eb6df729707bd7e0dd72e13e754caf9082e65303` |
| `GLS3/Source/glscs_execution_image.cpp` | `ae00f1cce875419b43213f645e53421b8c097e47038c6536051a8a798f27909a` |
| `GLS3/Source/glscs_compiler.cpp` | `4628b5c969f31cb8a5dc03464d567e2f30e172657cc1220dfb414f1156c10113` |
| `GLS3/Include/glscs_context.h` | `22687077bdd40f0e520ed46214e82069ef1a09f25b15599cdca135b76f59bfbc` |
| `ESL/Source/eslstring.cpp` | `136793de6a73503e4f775b8295afff7dcda1b5d5ef0763cc3e7881b685fff510` |

`native/runtime/cotopha_port/legacy_*.cpp` / `.h` / `.inc` 是项目的移植实现、桥接和诊断代码，其中有根据 GLS3 行为移植的算法与方法表（例如 `legacy_super_raster.cpp` 明确引用 `glssupsprite.cpp`，`legacy_sprite_methods.inc` 保留 GLS3 方法顺序）。它们不是由本次准备脚本从旧目录逐次复制的缓存。由于整个 GLS3 原始子树完全一致，没有发现仅因本次换源就必须改写这些 GLS3 适配的上游变化；此结论不等于证明所有手写适配正确，也不替代官方 GLS4 从源码构建后的运行验证。

## 构建隔离要求

审计开始时 `tools/sdk/prepare_legacy.py` 仅在 `build/legacy/GLS3/Source/glscs_context.cpp` 不存在时从旧压缩包解压；`cmake/legacy_runtime.cmake` 默认 `LEGACY_ROOT` 也指向 `build/legacy`。仅改某一个 SDK 路径不能保证彻底换源。

应让 `LEGACY_ROOT` 直接指向 `EntisGLS/EntisGLS4.07.03/EntisGLS3`（或从该新目录受控复制并记录来源），并在空的新构建目录重新生成 `legacy_generated`。旧 CMake cache 中的 `LEGACY_ROOT`、已生成源码、`.o`、`.a` 不能作为换源完成的依据。

同样需要核查独立 `native/compatibility/sdk/legacy/CMakeLists.txt` 及 probe 工具的 SDK 路径；Java、GLS4 ObjectHeap 生成器和其他 Cotopha 输入属于另一个审计范围。旧 archive 与旧缓存可保留用于历史比较，但新构建不应再依赖它们。

本审计仅写入本报告；未改写任一 SDK 或游戏资源目录。


## 迁移后只读复查与 probe 修正

复查时，主 APK 的新 `build/android-arm64` 329 个构建元数据文件和 `build/host` 53 个构建元数据文件中，未发现退役 SDK 的绝对路径或 `build/legacy/` 原始缓存引用。检查后缀为 `.txt`、`.make`、`.cmake`、`.d`、`.json`；数量为编译进行中的快照。`cmake/entis_sdk.cmake` 已强制让 `ENTIS_ROOT`、`LEGACY_ROOT` 指向新包，独立 legacy CMake 入口也使用该统一配置。

发现独立 `build/legacy-arm64` 仍保留旧 CMake cache 和旧 include 参数。五个 `compile_legacy_{audio_player,expression,input,movie,setup}_probe.py` 默认读取该旧构建，`compile_legacy_compiler_probe.py` 也硬编码旧构建。经主任务授权，已修改这六个工具：

- 默认读取当前 `build/android-arm64`。
- 共用 `tools/build/official_build_flags.py` 校验 `ENTIS_ROOT`、`LEGACY_ROOT` 必须为新包路径；发现旧 cache 或旧 SDK / `build/legacy` include 路径时直接失败。
- compiler probe 直接编译当前构建已经适配的生成源码，不再重复应用补丁，并正确返回编译失败状态。

未运行原生编译以免与主构建争用。已完成六脚本语法检查、真实新构建 flags 读取，以及临时夹具中的旧 cache 拒绝、伪装为新 cache 但旧 include 参数拒绝、正确官方配置接受四项检查，均通过。其余默认读取主构建目录的历史辅助脚本未在此次修正中改写。


后续扩大同一校验到 `compile_legacy_{volume_envelope,image_export,atomic_save}_probe.py` 和 `motion_compile_heap_probe.py`。heap probe 现在直接编译主构建配置生成的 GLS3 与 ObjectHeap 源文件，不再次复制或打补丁：原 `patch_legacy_heap_state.py` 对 `glscs_context.cpp` 的补丁需要未修改锚点，二次应用会报错。四个新增修正脚本语法检查通过，使用临时旧 cache 实际调用均在编译前以参数错误退出；当前 `entisgls4` 和 `legacy_objects` 编译参数检查均通过，未运行原生编译。
