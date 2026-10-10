# 词库更新日志

词库与 librime 补丁同仓、共用一个 tag —— 版本号只在本仓的 tag 上。

## dict-2026.10.10

简体拼音词库修订（纯数据，不涉引擎）。

### 机制：错音订正表

上游底本是钉死的 git 对象（`sources.lock`），错音改不了源头。新增
`tools/pinyin_fixes.txt`，由 `convert_dict.py` 生成时套用，复现门
`make check` 不变。三个指令：`fix` 整词替换读音、`del` 删一条错读音、
`add` 给单字补读音。契约写进 `dict/README.md`。

### 内容

全库 composability 扫描（词条每音节必须能由对应单字的读音组合而成）：
违例 125 条 → **35 条**（剩余为方言词与生僻字存疑项，逐条核过不收）。

- **48 处错音订正**：蚌埠市 bang fu→**bu**、家蚕 chan→**can**、
  国有财产 can→**chan**、港埠 fu→**bu**、中山高速公路 shu→**su**、
  甚麼 she→**shen**、褶子 xue→**zhe**、哪咤 nuó→**né**、
  阿毗昙 tian→**tan**、肛瘘 lv→**lou** 等。
- **3 处词条文本订正**（读音暴露了底本的词形错误）：
  安静自在→**安闲自在**、心焦如火→**心焦如焚**、灵桩→**灵椿**。
- **19 处单字补读音**：行 heng（道行）、挝 wo（老挝）、倘 chang（倘佯）、
  喋 zha（唼喋）、趵 bo（趵趵）、摘 ti（摘伏）、唫 yin（呿唫）、
  碌 liu（碌碡）、榜 beng（榜人）、平 pian（平章）、舚 tian、乣 diu、
  弸 peng、摵 she、踶 zhi、髿 suo、椆 diao、杕 duo、勺 zhuo（舞勺）。
  补读对齐上游自身词条口径（底本里 道行 dao heng 已存在而 行 缺 heng）。

`luna_pinyin.dict.yaml`: 67164 → **67176** 条（+68 读音、−56 读音，
净 +12）。`essay.txt` 与 `opencc/` 未动，逐字节复现通过。

## v1.1.9-rime-stack

两个数据 commit 之后词库内容已变，本次给当前内容一个版本坐标（此前最新 tag
`v1.1.8-rime-stack` 停在简体化之前，dict 内容无 tag）。

### 全部转简体

`essay.txt` 与 `luna_pinyin.dict.yaml` 原为繁体（上游 rime-essay / luna_pinyin 都是
繁体），靠 `luna_pinyin_simp` 的 simplifier 转简体上屏；现在两库内容本身已是简体，
simplifier 退化为幂等护栏。

- `essay.txt`: 442688 -> **437873** 条（转简体 + 按词去重取最大权重）
- `luna_pinyin.dict.yaml`: 70655 -> **67164** 条 (词,拼音) 对（按对去重，同词多读音保留独立行）

两个坑写进 `dict/README.md`：`乾`(gān/qián)、`薹`(tái) 是合法简体字但 opencc `t2s`
会误转，需逐字保护；一简对多繁需按 (词,拼音) 对去重而非只按词去重。

### essay 升级

`essay.txt` 升级到 rime-essay 官方最新版（442688 条），并修正授权标注：essay.txt
派生自 rime/rime-essay，按 LGPL-3.0 授权，其余文件为 MIT。

## v1.1.0-rime-stack.1

词库并入本仓（原先在 `adam-ikari/term-ime-dict`）：

- `luna_pinyin.dict.yaml` —— 70655 条字词
- `essay.txt` —— 297731 条词频
- `opencc/` —— 简繁转换数据

词库内容与 `term-ime-dict` 的 `v1.0.0` 完全一致，本次只是换了位置，没有改动任何
一个字节。目的是让「用哪个 librime」与「用哪个词库」变成同一个版本坐标。

并入前的独立版本记录见 git 历史里 `dict/` 之前的独立仓；词库自 v1.0.0 起的内容
未变。