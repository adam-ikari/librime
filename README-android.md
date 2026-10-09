# Rime with Android

Build the engine library (pure C API `librime.so`) for Android with the NDK.

The JNI bridge is **not** part of this repository: this library exposes the
upstream C API (`rime_api.h`, entry point `rime_get_api`), and the consumer
(Terminal/Trime/fcitx5-android style frontend) is responsible for its own JNI
layer. The dictionary toolchain (`dict/tools/`) is also not involved — this
build only compiles the engine; the runtime dictionary data (`dict/`) is
packaged by the consumer.

## Build

``` sh
# download the NDK once, e.g. r28
# https://developer.android.com/ndk/downloads

git clone --recursive https://github.com/adam-ikari/librime-stl.git
cd librime-stl

NDK=/path/to/android-ndk-r28 ./build-android.sh arm64-v8a   # or: x86_64, armeabi-v7a
```

Output: `build/android-<abi>/rime/lib/librime.so`

The script cross-compiles the five static deps
(glog/leveldb/marisa-trie/opencc/yaml-cpp) and then the shared engine library.
`opencc` is built with `OPENCC_BUILD_DATA=OFF OPENCC_BUILD_TOOLS=OFF`, so
dictionary generation never runs on the NDK build — see `dict/README.md`.

Verified (2026-10-09, NDK r28, no device):

- `arm64-v8a` and `x86_64` produce an ELF shared object whose `NEEDED` entries
  are bionic system libraries only (`liblog.so libm.so libdl.so libc.so`).
- `rime_get_api` is exported as an unmangled C symbol; a test consumer linked
  with `aarch64-linux-android21-clang++` resolves it against `librime.so`.
- **Not verified on a real device** — the binary has never run on Android
  hardware.
