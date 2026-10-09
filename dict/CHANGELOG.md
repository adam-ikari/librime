# 词库更新日志

词库与 librime 补丁同仓、共用一个 tag —— 版本号只在本仓的 tag 上。

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