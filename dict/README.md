# term-ime 词库

拼音词库与简繁转换数据。原先在独立仓 `adam-ikari/term-ime-dict` 独立版本维护，
现并入本仓的 `dict/` 目录 —— 与 librime 补丁同仓、共用一个 tag，rime 栈从此只有
一个版本坐标。

## 文件

| 文件 | 说明 |
|------|------|
| `luna_pinyin.dict.yaml` | 词库本体。67164 条 (词, 拼音) 映射（2026-10 全部转简体并去重，原 70655 条繁体）。同词多读音保留为多行。 |
| `essay.txt` | 词频表，437873 条带权重词条（2026-10 升级到 rime-essay 官方最新版 442688 条，再全部转简体去重）。词库开了 `use_preset_vocabulary`，靠它加载词频 —— 缺了它候选会退化成按 Unicode 排序的单字（生僻字排在常用字前面），词组也出不来。 |
| `opencc/` | 简繁与字形转换数据，供 schema 的 simplifier 用：`t2s_full.json`（简体输出主路径）、`t2hk.json` / `t2tw.json`（港/臺字形）。 |

三个文件是一套：schema 按名字引用 `luna_pinyin` 词库，词库按名字加载 `essay.txt`，
简体方案再挂 opencc 转换。改其中任何一个都要另两个配套。

## opencc/ 必须是自足的

librime 把 `opencc_config` 解析成 `<共享数据目录>/opencc/<配置文件>`
（`src/rime/gear/simplifier.cc`）；文件找不到时 `SimplifierComponent::Create`
返回 `nullptr` —— filter 静默不存在，候选退成未转换的字形，部署不报错、日志之外没有任何痕迹。

所以 `dict/opencc/` 里的每个 `.json` 所引用的 `.ocd2` / `.txt` 都必须和它一起提交，
不能靠 opencc 安装前缀兜底：`libopencc.a` 里的 `PKGDATADIR` 是编译期烧进去的绝对路径，
只在构建机上成立。本仓的 `CMakeLists.txt` 在 configure 期断言这条闭包，缺文件即失败。

**简体化（2026-10）**：两库原本是繁体（rime-essay 与 luna_pinyin 上游都是繁体），
靠 `luna_pinyin_simp` 的 simplifier 转简体上屏。现在两库内容本身已是简体，
simplifier 退化为幂等护栏。

转换有两个必须知道的坑：

- **`乾`（gān 干燥 / qián 乾隆）与 `薹`（tái 蒜薹）是合法简体字**，不是繁体
  残留，但 opencc `t2s` 会把它们误转成 `干`/`苔`。转换时逐字保护，否则输 `qian`
  会出「干」而不是「乾」。`tests/test_simplified_candidates.py` 的 KEEP 集合钉住了
  这条契约。
- **一简对多繁**：`乾` 有 gan/qian 两读、`乾/幹/榦/汫` 都归 `干`。转换按 (词, 拼音)
  对去重而非只按词去重，保留多读音独立行 —— 合并成单一读音会让输入另一读音时打不出
  该字。

`LICENSE` 按文件分别标注授权，`dict/` 有三种：`luna_pinyin.dict.yaml` 与
`opencc/t2s_full.json`、`opencc/variants*.txt` 是 MIT（Copyright (c) 2026
adam-ikari，本项目原创）；`essay.txt` 派生自 rime/rime-essay，以 LGPL-3.0 授权；
`opencc/` 里的 `t2s.json`/`t2hk.json`/`t2tw.json` 与 `*.ocd2` 派生自 BYVoid/OpenCC，
以 Apache-2.0 授权。整个 `dict/` 不是单一授权 —— 不能整体当作 MIT。

## 为什么并进来

原先词库与 librime 是两个 submodule、两个独立的 pin 点。librime 那边锁的是
`1.16.1` 之后的无名 commit（没有 tag），所以「term-ime 用的哪个 librime」无法
命名；词库这边只有 `v1.0.0`。两个坐标要靠人脑记住它们配套。

并成一个 tag 之后，词库数据与消费它的 librime 补丁同一次提交、同一个版本号，
不存在「装了新词库但 librime 没跟上」这种组合。

代价：本仓不再是干净的 librime fork。日后同步上游：

```sh
git remote add upstream https://github.com/rime/librime
git fetch upstream
git merge upstream/master        # dict/ 与上游无交集，正常不会冲突
```

`dict/` 是上游不存在的目录，所以合并只会碰 librime 自己的文件。

## 版本

版本号在本仓的 tag 上，词库与 librime 一起发。term-ime 用 submodule 锁定本仓的
某个 commit 发版，组合关系因此固定在 term-ime 那次 tag 里。

改词库：在这里改，随 librime 补丁一起发 tag，然后让 term-ime 更新 submodule 指针。