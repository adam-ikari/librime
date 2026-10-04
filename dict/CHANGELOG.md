# 词库更新日志

词库与 librime 补丁同仓、共用一个 tag —— 版本号只在本仓的 tag 上。

## v1.1.0-rime-stack.1

词库并入本仓（原先在 `adam-ikari/term-ime-dict`）：

- `luna_pinyin.dict.yaml` —— 70655 条字词
- `essay.txt` —— 297731 条词频
- `opencc/` —— 简繁转换数据

词库内容与 `term-ime-dict` 的 `v1.0.0` 完全一致，本次只是换了位置，没有改动任何
一个字节。目的是让「用哪个 librime」与「用哪个词库」变成同一个版本坐标。

并入前的独立版本记录见 git 历史里 `dict/` 之前的独立仓；词库自 v1.0.0 起的内容
未变。