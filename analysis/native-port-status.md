# 原生移植取证与当前边界

> 目录整理更新：文中历史路径 `EntisGLS/EntisGLS4.07.03/` 现已上移为 `EntisGLS/`；源码内容保持不变。`Primrose2` 与 `loquaty_lib_1.02` 发行目录已移除，构建仍使用 `vendor/official-loquaty` 源码。

历史真机记录日期：2026-09-23。设备：Mi 10 Pro，Android 13，支持 arm64-v8a。

## SDK 换源（2026-09-27）

当前源码依赖已切换为 `EntisGLS/EntisGLS4.07.03/`，SDK 和 Loquaty 均从源码重编，
不再使用原目录或其预编译库。完整 ARM64 编译、链接、APK 签名校验通过。
新 APK SHA-256：`2b7bc4b48d9f7453120c06cbde35d4cc1ba2688c9a66e39895dd6c2ee647bffe`。
绘制冻结兼容已移入项目适配层，8 项生产 gate 主机测试通过；SDK 原文件校验未变。
本次没有连接手机，未进行新 APK 的真机复测。下方验收、游戏进度及设备测试均属于
迁移前构建；原 APK/报告保留于 `artifacts/official-sdk-migration/previous-build/`。
迁移细节与验证边界见 `analysis/official-sdk-migration.md`。

## 用户验收（2026-09-23）

用户明确要求「不验了，把当前状态视为已达成目标」。按此验收当前 APK，停止后续测试。
当前已推进至 `nak_21`，真实保存分支前 slot21（71120 字节，上下文 93333 字节）。
此前 `nak_15` 选择前 slot20 的冷读与选择继续已通过。最后一个已启动的 cold21 辅助
流程在处理停止请求前已结束；未继续点击或检查其画面，不新增真机验证结论。

后续 Nak 片尾/后日谈、完成解锁持久化和其他完整路线仍未验证；验收不等于完整通关证明。
交付 APK 为 `artifacts/studysteady-arm64-dev.apk`，SHA-256：
`3ff957d4b8e93bccba86b0328a98a89a40b761ca6169296d3bf77163b4ec4b3b`。

## 最新验证（19:44）

当前 APK SHA-256：`3ff957d4b8e93bccba86b0328a98a89a40b761ca6169296d3bf77163b4ec4b3b`。
片头死锁修复已经过真实窗口完整播放验证：97332 ms 原始视频自然运行 97454 ms，
到达第 2919/2920 帧，记录 5310 次 GL 绘制和 2918 个不同视频帧，随后播放线程自然结束。
停止、关闭、重新打开再播放 3 秒也通过。日志 `artifacts/opening-full-regression.txt`，
画面证据 `artifacts/opening-full-middle.png`。此前两轮 3 秒回归在
`artifacts/opening-window-regression.txt`；完整原生自检 19:28:25 全部通过，
`artifacts/opening-fix-self-test.txt`（此后仅新增完整视频的诊断入口）。

原游戏已在快进中越过片头并推进到 `nak_07_1`。真实 UI 保存 slot13 为 71888 字节，
上下文 95825 字节；冷读、普通触摸对白和角色位置通过：
`artifacts/game-nak07-cold13.txt/png`，当前构建再冷读也通过。
快进流程不代表观看了整部片头，完整时长证据来自前述独立真实窗口回归。
当前继续验证路线中后段选择、片尾、后日谈及解锁；下方旧断连/ANR属于历史记录。

## 最新构建（10:05）

片头直接绘制死锁已改为 Android 正常精灵合成，保留原播放参数和存档标志。
新增 `--opening-probe` 真实窗口片头回归，原 MEI 状态测试增加 direct 位保存恢复检查。
ARM64 编译、APK 签名校验通过；新版 APK SHA-256：
`a36fd0f9d234036367a5a65176141d7172a317d768e078ca90d337f695367b75`。

ADB 安装返回 Success，片头探针启动请求已接受；随后手机从 ADB 与 macOS USB 清单
消失，未取得探针结果。原因未知，已请求重新连接，不能把本次测试标为通过。
原始完整片头及后续剧情回归仍待执行。健康辅助工具已加入当前 ANR 检测，8 项主机
回归通过，避免再次把有 ANR 的前台进程报成健康。

离线补充：全部 529 个剧情 XML 的 24 组影片调用中，8 组 direct 都指向 opening.mei，
其余 16 组列车/雪景循环影片实际把标志赋为 2，不包含 direct。
对应参数及证据见 `native/motion_bridge/evidence/movie_presentation_usage.json`。
这确定了复测范围，不表示影片已经真机通过。USB 断连保护增加 15 秒 ADB 超时，
保留原始错误与已有日志，本地模拟回归增至 9 项通过。

## 当前阻点（09:49）

实际流程已经从 common1_8 的 save0012.bmp 继续到片头视频，但片头出现 Android ANR。
当前 APK SHA-256 为 `d51d23c33db91f88cf2132fd2d38cf4f775730811cc3df0537c1692c657d2e00`。
主线程等待 GLSurfaceView monitor，GL 线程等待原生全局锁，视频线程持锁同步等待 GL 任务。
完整现场在 `artifacts/opening-anr-traces.txt`，系统记录在 `artifacts/opening-anr-dropbox.txt`。
正在修复视频直接绘制路径；此前小型 MEI 自检通过不能代表完整片头已经通过。
尝试保存 slot13 时暴露无响应，slot13 没有写出；最近有效手动检查点仍是 slot12。

之前新增的真机验证已通过：common1_7_1 与 common1_8 地图冷读档、历史记录滚动、
原版回溯确认与上下文恢复；86 个 PSB 均完成真实 GLES 基础姿态初始化和释放检查。
这仍不代表全部角色时间线或完整路线验证完成。

## 最新验证（09:26）

下面按阶段保留的早期日志属于历史结果。小米 10 Pro 已运行原版繁体中文标题、
开场 MEI、姓名和正文，触摸推进经过首个选择（选「剑道」）、便利店双角色剧情，
已进入共同线 common1_5 地图，并选择宠物店分支。原版设置菜单能打开、切换页并返回；
切到安卓桌面再返回，进程、画面和输入正常。

游戏存档已完成真实链路：昵称语音后保存 save0014.bmp（70390 字节），正常菜单退出，
重新启动从标题读取、继续对白，再保存 save0015.bmp（70696 字节）。
正常退出也写出了 sysenv.dat（553 字节）。双角色 save0016.bmp（70924 字节）
已从标题冷读；继续对白和转场后保存 save0017.bmp（70876 字节），再次冷读成功。
原游戏资源未改，旧调试格式存档已独立备份。

关键修复与证据：

- 恢复原 Array/Hash 允许静态脚本缓存暂时未恢复的行为。原 CSX 的 OnContextLoaded
  会重载缓存，再退出旧 XML 帧；实际冷读档通过。
- AudioPlayer 按原 EXE 保存原 MIO 和昵称替换说明，读档实际重新拼接音频。
  真实游戏保存/再保存通过，137 字节记录和完整 PCM 比较、失败清理通过。
- 活动中的淡入淡出和运动曲线支持原版分割、替换、继续及保存恢复；
  实际文字框更新已越过原失败点，Sprite draw 探针通过。
- FilterLight/White/Black/Dark、RasterScroll、ShadingOff/Light 已补齐原像素行为，
  真机验证格式、alpha、当前帧、源图不变，以及 SuperSprite 保存恢复和复制。
- bgm05.mio 的大文件流式播放、位置推进、停止、重开真机两轮通过。
- E-mote 绘制前以零时间发布控制器值，修复冷读档首帧角色纵坐标；
  图像更新用 NotifyUpdate 通知父层，修复动画画面不持续刷新。
  实际双角色冷读后位置正确，两个角色均达到第 31 次绘制，像素哈希发生变化。
  真实 GLES 独立回归同时验证首帧、重复零时间像素、时间线、状态和共享纹理。

完整原生自检于 09:02 全部通过：`artifacts/dynamic-dialog-self-test.txt`；包含新增的
Sprite.EnableDynamicMode 真实像素、裁剪/透明变换回退、保存/恢复等 17 项检查。
之前的效果回归日志为 `artifacts/all-effects-self-test.txt`。
之后的 E-mote 修复另通过 `artifacts/motion-cold-first-frame-gles-arm64.txt`，
实际游戏证据见 `game-actors-cold-fixed.txt/png`、`game-actors-continued-fixed.txt`、
`game-actors-resave17.txt`、`game-actors-cold17.txt/png`。

游戏原生截图功能已接 Android SDK 文件框，取消和实际保存通过；生成 6220854 字节、
1920×1080 BMP，完整解码及目视检查通过（`artifacts/game-native-export.bmp`）。

地图停留曾被误判为残留，严格复验已排除：原脚本快进时首个点击仅取消快进，
第二次点击才确认地点，然后宠物店有约 18 秒镜头流程。
`artifacts/game-map-skip-correct-confirm.txt/png` 记录正确解绑与后续正文；没有为此更改事件或渲染语义。

当前仍在验证更多真实剧情、角色、转场和操作；尚未验证完整路线，不能标为任务完成。
默认 APK 启动直接执行原游戏；`--self-test`、`--emote-probe`、`--image-export-probe`
是独立开发自检入口。`tools/device_game_smoke.py` 可在本机 2340×1080 设备上重复
标题读档/新游戏/推进流程，保留日志，并在主脚本退出时停止后续点击和截图。

## 输入

| 文件 | SHA-256 |
| --- | --- |
| ststeady.exe | 9750f67ae51ba158cda4f756ccbd0875fb837ddd15e84db713be53f8573d91b3 |
| emotedriver.dll | 40d2e510106fc8a79e0fd07e5c0d4839d75503089d6fc9de0f0904f0ab73bd4d |

原始游戏和引擎目录未修改。IDA 在 `build/analysis` 的副本上工作。

## NOA 与 PSB 加密

根目录 NOA 索引已读取。编码 0 为原始存储，`0x80000010` 为 ERISAN 压缩；
它们与 `0x20000000` / `0xA0000010` 的加密编码不同。
已检查根目录条目未出现 NOA 加密模式。嵌套 NOA 未全部递归检查。

`script.csx` 解压后为 2,017,818 字节的 Cotopha 模块。
`IDR_COTOMI` 解压后为 1,350 字节的启动配置。

`psb.noa` 中的 68 个 PSB 自身是 v4、header flags 1。头部 `[8,56)` 的
xorshift 流种子 851083516 通过已知头大小提出候选，并用每个实际文件存储的
Adler-32 校验独立验证。校验覆盖 `[8,40)` 与 `[44,56)`。
该种子不是遍历所有密钥的结果，也不是假定旧插件自动支持 v4。

`haz_a.psb` 解析为 `id=motion, spec=win, version=3.03`，21 个 object 分组，
109,445 个节点；存在 264 个 tag 0x22 和 14 个 tag 0x23 的扩展资源引用。
旧插件未正确处理这些 v4 引用，因此另加了带范围校验的读取边界。

全量检查结果：16 个 NOA、10,795,116,014 字节；基础角色 68 个与 R18
补丁角色 18 个，86/86 个 PSB 通过头部校验、对象树和扩展资源引用遍历。
完整逐文件结果在 `artifacts/game-audit.json`。错误密钥和截断文件被拒绝。

## 游戏驱动证据

以下地址仅属于上述 SHA-256 对应的 Windows `emotedriver.dll`，image base
`0x10000000`。它们不是 kirikiroid2 的四个参考二进制的地址。本任务使用现有
MotionPlayer 实现作移植输入，没有改写源项目的四文件还原工作。

- `sub_10058680`：PSB v3/v4 检查；flags bit 0 时解密 36 字节基础头，v4
  继续解密 12 字节扩展头；验证 Adler-32；bit 1 对正文另行解密。
- `sub_100018E0`：四状态 xorshift 字节流；剩余密钥字为零时生成下一字。
- `sub_10057B00`：0x22..0x25 与普通资源一起归为资源类别 5。
- `sub_10057990`：0x22..0x25 读取 1..4 字节的小端资源索引。
- `sub_10057E70`, `sub_10057EC0`, `sub_10059070`：区分普通/扩展资源，
  从相应 offset/length/data 表取得内容。

已通过本轮原生 IDA MCP decompile 调用核实并保存 IDB。

## 真机结果

`artifacts/first-run-logcat.txt` 记录首个 APK 的执行：ARM64 运行库启动，
Environment 与 Primary module 均加载成功。SDK 日志将入口显示为 `test`，
但实际解析 `funcinfo` 后确认该地址为 `main`。进入入口后，
`ip=01000000:00005460` 抛出 `object mode` 异常。

`artifacts/psb-probe-arm64.txt` 记录独立 ARM64 原生程序读取实际 `haz_a.psb`，
遍历整个对象树、核对 v4 扩展资源范围，并执行导入的 MotionPlayer 插值。
这些检查通过，不代表角色渲染或游戏执行通过。

`artifacts/integrated-app-logcat.txt` 另验证 APK 内的 JNI 解析入口：实际 PSB
头部/对象读取与 MotionPlayer 插值通过，随后游戏脚本仍触发 object mode。
验证结束后已停止开发版应用；原游戏文件、源项目与手机其他应用未修改。

`artifacts/game-sync.json`：16 个 NOA 已复制到应用自己的外部存储目录，
逐文件 SHA-256 与原始包完全一致。`artifacts/archive-runtime-logcat.txt`
验证14个启动归档按原次序打开、CSX从压缩归档直接加载、传统文件桥完成
内存文件复制/截断/扩容、4 GiB以后的定位以及存档根目录的写入、读取和清理。
首次 SDK mkdir 创建的目录缺少 owner execute 位，已由 Activity 在初始化前
创建/修复自己的 savedata 根目录为0700；修复后的日志无该目录权限错误。

`artifacts/motion-gles-arm64.txt`：Adreno 650 对真实 `haz_a.psb` 的 4096² RGBA8
图集上传及逐像素回读通过，CRC32=`9d5638c1`，120个图块范围通过。
仍需完整节点求值、Bezier形变、遮罩、层级和动画。`motion-haz-a-pose-audit.json`
刻意标注 `renderable=false`，不把候选绘制列表当作角色渲染成功。

`artifacts/motion-tjs-arm64.txt`：从用户提供项目筛选复制的真实 TJS2 运行时、
真实parser和Oniguruma已在ARM64执行Dictionary/Array等脚本测试。它是完整
MotionPlayer依赖移植的基础，尚未表示Player已与Entis图层连接。

`artifacts/csx-startup-audit.md` 记录实际入口、动态归档顺序与跨架构文件布局。
主机C++验证251个cast、5017个方法记录、19273个UTF-16字符串重新编码与
原文件字节一致。传统核心对象、Context、ExecutionImage、线程和文件类已
链接进APK；动态 `String.Calculate` 已发现6个调用点，最终仍需移植表达式编译器。

## 传统运行时接入后的真机结果

`artifacts/legacy-runtime-logcat.txt` 首次证实传统ReadExecution在ARM64读取
553,952字节代码、170类、2,858字符串，入口0x5460；随后明确报告尚未接入的
Resource/Sprite平台类，而不是交给Sakura2直接执行对象指令。

Resource/Sprite基础实现已随后接入，`artifacts/legacy-core-logcat.txt`记录：
实际 `particle_light1.eri` 解码6×6/32bit、`se517.mio` 解码22050Hz/1504样本，
静音播放/停止成功；Sprite创建实际图像、修改位置/透明度/可见性、添加/拆分
父子关系通过。它们不包含窗口输出、听感或角色动画验证。

核心回归发现并修复：`obj.store 0xff`的枚举转换、对null对象调用非静态方法
导致的Clang优化崩溃、UTF-16与wchar_t宽度差异、ERISAN写缓冲未Flush，以及
解码后直接写内存缓冲却未提交长度的问题。引用生命周期、64位整数、字符串
内存桥、对象压缩保存/读取和真实NOA打开关闭现已在手机通过。
完整游戏存档兼容性仍未验证；Resource保存恢复、缩略图等接口仍明确报未实现。

最终同一次APK真机运行还通过了线程执行返回42、重复启动、启动后立即暂停、
恢复、终止、重复终止、终止后重启，以及递归互斥。修复了启动阶段覆盖外部
暂停/退出请求的竞争，用持久请求、控制互斥和配对原子读写保证状态传递。
之后ERI/MIO/Sprite探针全部通过，完整CSX重新加载成功；该阶段的精确阻塞为
`InitializeContext: InputFilter クラス情報を初期化できませんでした`。
游戏尚未进入main或标题画面。最终开发APK已签名安装，测试后停止应用。

## 窗口、输入与文字显示接入后的结果

`artifacts/native-binding-logcat.txt`：显式deferred-native元数据及按真实接收对象
解析已通过未使用类声明、严格构造错误、继承/保存的函数指针、普通脚本函数和
native opcode测试。CSX保存时恢复可移植的原生声明，不保存内部deferred枚举。

`artifacts/window-probe-logcat.txt` 与 `artifacts/window-probe.png`：2026-09-23
03:25真机由独立 `--window-probe` 入口创建1920×1080原生窗口，读取原始
`wm_langpicker.noa` 的5个图像资源和页面，实际显示语言选择画面。触摸日本語
按钮后收到 `ID_LANGPICKER_JA`，通过窗口/皮肤/触摸/旧命令队列的完整链路。
这是独立验证入口，不能当作原始main已经运行到语言选择页面。

`artifacts/message-setup-logcat.txt`：03:31同一APK中验证原始input.xml映射、
Alt修饰键、虚拟按钮、失焦释放、输入存取/等待；真实Android音量读取；
Setup摘要/编码/对话框按钮映射；日文、换行、ruby实际字体光栅、逐字推进、
Flush/Clear全部通过。APK已切换为单一真实TJS版PSB读取器与配套分配器。
完整CSX现在停在实际 `EmoteDevice` 对象创建。仍未进入真实main或标题画面。

`artifacts/motion-mesh-arm64.txt` / `motion-timeline-arm64.txt`：真实TJS/Player
读取haz_a，78组合节点/278轴/1112次变量更新通过；45 motions/379 nodes/
2940次正反向时间线求值通过，78节点正确发布和清除组合patch。
`motion-mesh-android.bin` 与 `build/motion-mesh-oracle/windows.bin` 逐位相同，
覆盖3505组实际/边界数据。完整层级、帧推进和角色GLES绘制仍在实现。

真实MotionPlayer CPU对象层另在手机构造26节点、13个子Player，三个默认参数
move_UD/move_LR/body_slant均为30，见 `artifacts/motion-player-arm64.txt`。
主机来源校验462/462、功能与负例7/7通过。Player未绘制/推进动画；实际角色
资源有额外meshCombinator数据，原导入实现没有读取，不能忽略该兼容缺口。
同一7项还在ASan/UBSan下通过，包括25轮真实Player/子Player构造析构，见
`artifacts/motion-verification-asan.json`。主APK尚未链接该独立TJS Player目标。

## 运行时来源和后续工作

术语澄清：“旧版”仅指沿用的传统 Cotopha 对象执行架构，不表示已经确认
游戏的 ECSContext 比 4.07.03 包内的 ECSContext 新或旧。二者的精确实现差异
尚未完成比对。4.07.03 是整个 SDK 包的版本，不能当作 ECSContext 的独立版本号。

随包 `EntisGLS3/EntisGLS3.7z` 内的 `GLS3/Source/glscs_context.cpp` 有旧对象
指令解释器（ExecuteNew/Load/Store/Call 等），`ExecuteInstruction` 根据最高位
切换 object/naked 模式；SDK默认安卓 `ECSSakura2::EnvironmentVM` 没有这一完整
对象层。本项目已生成并编译传统对象源码，Android环境继续负责资源，
`ECSContext`负责传统指令。线程、文件、基础媒体和精灵已有移植实现，
窗口、输入、其余内建对象及完整动画仍需继续接通。
不能通过忽略异常、跳过入口或只补一个 E-mote DLL 名称解决。
