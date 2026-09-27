# EntisGLS 依赖迁移（2026-09-27）

> 目录整理更新：文中历史路径 `EntisGLS/EntisGLS4.07.03/` 现已上移为 `EntisGLS/`；源码内容保持不变。`Primrose2` 与 `loquaty_lib_1.02` 发行目录已移除，构建仍使用 `vendor/official-loquaty` 源码。

当前构建已改用 `EntisGLS/EntisGLS4.07.03/`，完整 ARM64 编译、链接和 APK 签名校验通过。
新 APK：`artifacts/studysteady-arm64-dev.apk`，50,442,858 字节。
SHA-256：`2b7bc4b48d9f7453120c06cbde35d4cc1ba2688c9a66e39895dd6c2ee647bffe`。
本次没有连接 Android 设备，因此没有安装或真机复测；旧 APK 的游戏验证结果不能移用于新包。

## 构建输入

- SDK 的 290 个 Android 源文件按官方 Android.mk 清单编译为 libgls4.a；不再链接旧包内的预编译库。
- 原包没有 Loquaty C++ 源码和 TinyGLTF；从各自官方上游获取固定版本到 vendor/official-*。
  版本、commit、压缩包和源码树校验值在 `vendor/official-entis-dependencies.json`。
  Loquaty 的 42 个源文件已全部重新编译。
- 传统 ECS/CSX 源码直接读取新包已展开的 EntisGLS3/；199 个原文件与原归档内容相同，
  不再依赖旧 build/legacy 或旧 CMake cache。
- Java、JNI、头文件、主机解包工具和 ObjectHeap 生成器均改为新输入。
  APK 打包会重新生成 Java/classes/dex，独立 probe 会拒绝旧 SDK 缓存。

## 必要的项目适配

旧包增加了官方源码没有的 FreezePaint/UnfreezePaint 接口及窗口字段。
现改为项目内 LegacyPaintGate 与生成的 VirtualWindow JNI 入口，冻结时在获取 UI 锁前返回，
保留嵌套冻结及关闭清理行为；不修改官方 SDK 布局或原文件。
ObjectHeap 的已有读写检查仍在生成副本中实现。
详细比对见 `official-sdk-differences.md`、`official-sdk-legacy-audit.md`。

## 验证与保留内容

- 主机解包工具重新编译，原始 script.csx 和启动配置提取成功。
- 实际生产 paint gate 的 8 项主机测试通过，覆盖冻结/非冻结的 UI 锁时序、回调重入、嵌套及关闭。
  Android 自测新增对应检查但尚未在设备执行。
- 全新 SDK、传统对象运行时、Motion/TJS、JNI 和最终 APK 构建通过；签名校验通过。
- 审计 879 个构建元数据文件，没有退役 SDK 或旧 build/legacy 输入路径。
- 新包目录的 1805 个文件与本次迁移前快照全部 SHA-256 一致。
- APK 内原生库 SHA-256 与本次新编译产物一致；静态库与源码构建记录一致。
- 原 SDK 目录、游戏原资源未修改；旧 APK 和验证报告保留在
  `artifacts/official-sdk-migration/previous-build/`，旧构建目录保留为 `build/before-official-*`。

构建日志：`build/official-entis-build.log`、`build/official-legacy-build.log`、
`build/official-motion-build.log`、`build/official-apk-native-build.log`、`build/official-apk-build.log`。
审计与哈希：`artifacts/official-sdk-migration/dependency-audit.json`。
