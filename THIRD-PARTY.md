# Third-party licenses

This repository's own work is MIT (see [LICENSE](LICENSE)). It contains, or is
derived from, works under the following licenses. Their notices are retained.

## rime/librime — 3-Clause BSD License

The engine under `src/`, `include/`, `plugins/`, `test/`, `tools/` is a fork of
[rime/librime](https://github.com/rime/librime), Copyright (c) 2014, RIME
Developers, distributed under the 3-Clause BSD License (the text that was the
root `LICENSE` before this fork relicensed its own work to MIT):

```
Copyright (c) 2014, RIME Developers
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

  * Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

## dict/ data

See [dict/LICENSE](dict/LICENSE) for the per-file breakdown (LGPL-3.0 for
`essay.txt` and `luna_pinyin.dict.yaml`, Apache-2.0 for `opencc/`).

Full texts of the data licenses are vendored under [`licenses/`](licenses/):
`LGPL-3.0.txt`, `GPL-3.0.txt`, `Apache-2.0.txt`.

## Bundled dependencies

`deps/` are git submodules, each under its own license (Boost Software License,
BSD, Apache-2.0, LGPL-2.1, MIT, ...). See the linked projects in the README
credits.
