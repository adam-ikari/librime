# Third-party licenses

The engine is a fork of [rime/librime](https://github.com/rime/librime), Copyright (c)
2014, RIME Developers, distributed under the 3-Clause BSD License — the text in the root
[LICENSE](LICENSE), kept verbatim from upstream. The `dict/` data is a separate matter
with its own sources and licenses; see [dict/LICENSE](dict/LICENSE).

## dict/ data

Per-file breakdown (LGPL-3.0 for `essay.txt` and `luna_pinyin.dict.yaml`; Apache-2.0 for
the OpenCC-derived `*.json`/`*.ocd2`; MIT for this project's own `variants*.txt` and
`t2s_full.json`) is in [dict/LICENSE](dict/LICENSE). Full texts of the data licenses are
vendored under [`licenses/`](licenses/): `LGPL-3.0.txt`, `GPL-3.0.txt`, `Apache-2.0.txt`.

## Bundled dependencies

`deps/` are git submodules (plus the vendored `opencc`), each under its own license
(Boost Software License, BSD, Apache-2.0, LGPL-2.1, MIT, ...). See the linked projects
in the README credits.
