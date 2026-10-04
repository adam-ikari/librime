# term-ime 词库

拼音词库与简繁转换数据。原先在独立仓 `adam-ikari/term-ime-dict` 独立版本维护，
现并入本仓的 `dict/` 目录 —— 与 librime 补丁同仓、共用一个 tag，rime 栈从此只有
一个版本坐标。

## 文件

| 文件 | 说明 |
|------|------|
| `luna_pinyin.dict.yaml` | 词库本体。70655 条字词 → 拼音映射。 |
| `essay.txt` | 词频表，297731 条带权重词条。词库开了 `use_preset_vocabulary`，靠它加载词频 —— 缺了它候选会退化成按 Unicode 排序的单字（生僻字排在常用字前面），词组也出不来。 |
| `opencc/` | 简繁转换数据，供 `luna_pinyin_simp` 方案的 simplifier 用。 |

三个文件是一套：schema 按名字引用 `luna_pinyin` 词库，词库按名字加载 `essay.txt`，
简体方案再挂 opencc 转换。改其中任何一个都要另两个配套。

`LICENSE` 是这些数据的授权（MIT，Copyright (c) 2026 adam-ikari），与 librime 本体
的 LGPL 分开。

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