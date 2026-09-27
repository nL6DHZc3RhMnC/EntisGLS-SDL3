# SDL3 繁中缺字与正文退出修复

版本：0.2.1（构建号 3），2026-09-27。用户确认 Mac 与 Android 都存在问题；本轮按用户要求先验证 Mac，Android 只构建和验包。

## 根因与复现

原脚本繁中分支将 `config.sMsgFontFace` 设为 `Noto Serif CJK TC`；日文/英文使用 `MsgFont`。姓名、昵称与正文均读取此配置。仍可见的固定标签多为 ERI 图片。

原游戏附有 `SETUP_ZHTW/NotoSerifCJKtc-Bold.otf`，说明文件要求安装字体。旧 SDL 后端仅接入 BMF，没有处理这个 OpenType/CFF 字体。现有 BMF 还缺「說」「腳」「夠」，不能简单将 Noto 名称映射到 BMF。

`build/sdl-zhtw-regression/lldb-before-retry.log` 记录了实际失败：首段请求 `Noto Serif CJK TC/56`，输入 50、消费 3、字形 0；Cotopha 在 `0x0000dc8d` 抛运行时错误，最终未捕获异常、进程退出码 **6**。本次重现为脚本错误导致退出，没有捕获到原生 SIGSEGV。

## 修改

- `native/platform/sdl/opentype_font.cpp` 使用共用 FreeType 后端注册真实字体。每实例独立持有 library/face，共享不可变字体字节，不依赖宿主安装字体。
- 优先读取包内 `assets://fonts/NotoSerifCJKtc-Bold.otf`。SDK 字号是字符单元高度，使用 `FT_SIZE_REQUEST_TYPE_REAL_DIM` 保持语义，避免文字裁切。
- 支持当前游戏所需灰度、单色、斜体和高质量采样；拒绝非法大小/码位、缺失字形、不足缓冲区及未实现样式。日文 BMF 路径保留。
- 新增固定版本 [FreeType 2.14.3](https://freetype.org/) 与原样复制的 Noto 字体，完整许可证随包提供。`tools/setup_sdl_fonts.py --verify` 离线核对输入，不覆盖已有内容。
- Mac/Android 打包时校验包内字体 SHA 及许可证；只导入 NOA 的安卓用户也可直接用繁中。
- 顺带修复实际复现的姓名确认点击丢失：click 后同 ID 的状态通知可能覆盖点击。保护扩展到原生鼠标/触控回调和悬停重放；公开脚本 `QueueCommand`、键盘和范围外命令保留原覆盖语义。

SDK、游戏原目录及既有 vendor 输入未修改。FreeType 在新目录 `vendor/freetype`，字体复制到 `assets/fonts`；旧 SDL 包保存在 `artifacts/sdl3/before-zhtw-fix`。

## 验证

- 新字体自测：33 个繁中字、16/40/56px、行高边界、空格、样式、非法输入、短缓冲区、并发不同字号与生命周期全部通过。
- 完整 legacy 自测通过，包括日文 BMF、对象/媒体/文字/存档和新增的鼠标按钮回归。
- 最终 Intel ZIP 解压后严格验签并实际运行，通过繁中姓名/昵称页面、姓名确认与前三段正文。姓「山田」、名「太郎」和六个昵称均已目视确认可见；两次 Enter 后正文仍正常，退出码 0。
- 最终测试使用仅含 16 个 NOA 链接的游戏目录，排除了外部 `SETUP_ZHTW` 字体兜底；测试存档在临时目录隔离。日志确认字体确实来自包内 assets。
- ARM64 Mac：完整交叉构建、字体/许可证字节、CRC、解压签名与架构检查通过，未在 Apple Silicon 上运行。
- Android：版本 `0.2.1-sdl3-dev`/versionCode 3；完整构建、字体/许可证 SHA、签名、ZIP/ELF 16 KB 对齐及动态依赖检查通过；本轮未安装或真机运行。

最终 Mac 证据在 `artifacts/sdl3/zhtw-fix/runtime-verification.json`、`selftest.log`、`name.png`、`dialogue.png`；复现命令为 `python3 tools/verify_sdl_zhtw_macos.py`。Android/ARM64 验包报告在 `build/sdl-zhtw-regression/`。

未重跑完整路线、长时间运行或中文系统输入法。本次覆盖姓名/昵称缺字、首段正文退出及紧接其后的对白推进。

## 最终产物

| 平台 | 路径 | 字节数 | SHA-256 |
| --- | --- | ---: | --- |
| Intel Mac | `artifacts/sdl3/macos-x86_64/StudySteady.zip` | 30,412,260 | `63b24574afddf59f37cce3e821960126e37ace5f9a407e56b408bc632fe0f95d` |
| Apple Silicon Mac | `artifacts/sdl3/macos-arm64/StudySteady.zip` | 29,332,015 | `5b6bfd84a4044566a717f96a34099545e75e2ca10d097b210c6a7fd08e2a3bae` |
| Android ARM64 | `artifacts/studysteady-sdl3-arm64-dev.apk` | 76,191,095 | `7b954e36c943244aed16bfd14cbb50b7e801acba138fa0bd01505a6814ccef9d` |

包增大来自内置的 24,684,480 字节繁中字库，仍不包含 NOA。Mac 交付已解压验签的 ZIP；相邻 `.app` 有先前记录的 Documents/FileProvider FinderInfo 问题。

字体 SHA-256 为 `d6bc09d324004b38207898f86deb298cc4eb0527bc40916f25ba9d3ba07226f9`。字体许可来源见 `assets/fonts/provenance.json`；FreeType tag、commit 与 744 文件摘要见 `vendor/freetype-provenance.json`。
