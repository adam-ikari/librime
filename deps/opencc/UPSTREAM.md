# OpenCC, vendored

`deps/opencc` 是 [BYVoid/OpenCC](https://github.com/BYVoid/OpenCC) 的
`ver.1.1.9`（commit `556ed22496d650bd0b13b6c163be9814637970ae`）的副本，不是
submodule。原先它是指向上游的 submodule，而 librime 需要给它打补丁 —— 为了不在
rime 栈里再多出一个 fork，把它直接收进本仓。

授权照旧：本目录内是 OpenCC 的 Apache-2.0 内容，见 `LICENSE`。

## 本地改动

只有构建开关，没有动任何逻辑：

| 文件 | 改动 | 原因 |
|---|---|---|
| `CMakeLists.txt` | 新增 `option(OPENCC_BUILD_DATA ON)`，用它罩住原来的无条件 `add_subdirectory(data)` | `data/CMakeLists.txt` 第一行就是 `find_package(PythonInterp REQUIRED)`，并且把 `.ocd2` 生成挂在 `ALL` 目标 `Dictionaries` 上、构建时**执行** host 的 `opencc_dict`。引擎不需要这些生成物：librime 从 rime 共享数据目录读 `.ocd2`（`dict/opencc/`），不读 opencc 的安装前缀 |
| `src/CMakeLists.txt` | 新增 `option(OPENCC_BUILD_TOOLS ON)`，罩住 `add_subdirectory(tools)` | 交叉编译时这三个工具是目标机二进制，却在构建机上被执行 —— `Exec format error`。NDK/Android 的头号阻塞 |
| `CMakeLists.txt` | `OPENCC_BUILD_DATA` 开而 `OPENCC_BUILD_TOOLS` 关时 `FATAL_ERROR` | 生成 `.ocd2` 用的就是 `opencc_dict`，这个组合只会在构建中途炸出难读的错 |

两个开关默认都是 `ON`，所以按上游习惯构建（`make deps`、直接 `cmake`）行为不变。
librime 与 term-ime 显式传 `OFF`，把词库生成完全请出引擎的编译图。

## 同步上游

```sh
git -C /tmp/opencc clone --depth 1 https://github.com/BYVoid/OpenCC.git
# 用上游内容覆盖本目录，保留 UPSTREAM.md，然后重放上面三处改动
rsync -a --exclude=.git --exclude=UPSTREAM.md /tmp/opencc/OpenCC/ deps/opencc/
```

覆盖完先跑 `git diff` 看三处开关有没有被冲掉，再按 `dict/README.md` 的门禁验证
引擎仍然独立构建通过。
