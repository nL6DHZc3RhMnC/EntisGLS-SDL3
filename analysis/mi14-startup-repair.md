# 小米 14 Pro 启动修复（2026-09-23）

设备 `23116PN5BC` / shennong，Android 16 / API 36，arm64-v8a，4 KB 内存页。
当前 APK SHA-256 与已交付版本一致：
`3ff957d4b8e93bccba86b0328a98a89a40b761ca6169296d3bf77163b4ec4b3b`。
本次没有重装 APK、清除应用数据或修改两个原始目录。

## 已确认并修复的资源权限问题

原有 16 个 NOA 位于正确的 `files/game/` 目录，大小与原始资源相符，
但游戏进程逐个打开时均收到 `EACCES` / `Permission denied`。
旧文件 stat 为 UID 10254（`com.android.providers.media.module`）、GID 1023、0660；
游戏 UID 为 10716。仅凭目录存在、文件大小正确不能推断游戏可读。
没有原导入过程记录，不能断定具体是哪个工具或步骤导致此权限状态。

先备份并重建 `patch2.noa`；不改变 APK，真实游戏日志立即由 EACCES 变为成功挂载，
而旧 patch1/script 仍不可读。随后全量采用先上传新文件并校验 SHA-256、再备份旧文件
并替换正式路径的方式修复。新文件 stat 为 UID 2000、GID 1078、0666。
全部 16 个文件 SHA-256 与电脑原始包及此前记录一致；所有旧文件仍保留为隐藏备份。
备份共 10,795,116,014 字节，占用约 10.8 GB 额外空间；没有清除存档。

修复后真实应用 PID 23522 在 21:42:26 成功挂载全部 16 个 NOA，
包括脚本动态挂载的 R18 补丁和 voice.noa，原始 CSX 的 170 个类、2858 个字符串
加载成功，进入 main，已加载语言选择 skin。该次日志中没有 EACCES。

## 待确认的画面启动

上述启动同时出现 Activity onPause/onStop、Surface 销毁，之后 GL context 为空；
捕获画面为全黑竖屏。随后 USB 断开，尚未在解锁且前台状态下重新启动验证。
因此当前证据证明资源权限已修复，不等于标题画面和触摸已通过。
已请求重新连接并解锁手机，仅继续本次启动排障，不恢复完整路线测试。

## 证据与工具

- `artifacts/mi14-startup/existing-pid-log.txt`：原游戏进程 EACCES。
- `artifacts/mi14-startup/patch2-reimport-log.txt`：单包重建成功对照。
- `artifacts/mi14-startup/repair-resources.json`：16 包 SHA、stat 和备份路径。
- `artifacts/mi14-startup/repaired-startup-log.txt`：16 包真实应用挂载和生命周期。
- `artifacts/mi14-startup/apk-verification.json`：手机 APK 与交付 APK 的哈希一致。
- `tools/sync_game.py --repair-permissions --report <路径>`：可重复使用的保留备份修复选项。
  上传中断、校验失败、替换失败、恢复失败、断连及意外目标保护等 11 项主机测试通过；
  脚本自身只验证 shell 哈希，真实应用访问必须另行检查。
