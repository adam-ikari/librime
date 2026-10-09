#!/bin/sh
# 交叉编译 Android 输入法引擎库：纯 C API 的 librime.so。
#
# 词库工具链（dict/tools/）不参与 —— 这里只编引擎，运行时词库数据（dict/）
# 由消费方打包（见 dict/README.md）。opencc 传 OPENCC_BUILD_DATA=OFF 和
# OPENCC_BUILD_TOOLS=OFF：生成数据需要 host Python + 现编译的 host
# opencc_dict，交叉编译下直接 Exec format error，详见 deps/opencc/UPSTREAM.md。
#
# 用法: NDK=/path/to/android-ndk-r28 ./build-android.sh [arm64-v8a|armeabi-v7a|x86_64]
# 产物: build/android-<abi>/librime.so
set -eu

ndk=${NDK:?set NDK to the Android NDK root, e.g. NDK=/opt/android-ndk-r28}
abi=${1:-arm64-v8a}
platform=android-21
toolchain="$ndk/build/cmake/android.toolchain.cmake"
[ -f "$toolchain" ] || { echo "no toolchain file at $toolchain" >&2; exit 1; }

rime_root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build="$rime_root/build/android-$abi"
prefix="$build/prefix"

# CMAKE_FIND_ROOT_PATH must include $prefix: the NDK toolchain file sets
# FIND_ROOT_PATH_MODE_LIBRARY=ONLY, so the engine's find modules would otherwise
# only see the sysroot and never the static deps we install there.
common="-DCMAKE_TOOLCHAIN_FILE=$toolchain -DANDROID_ABI=$abi -DANDROID_PLATFORM=$platform \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$prefix \
  -DCMAKE_FIND_ROOT_PATH=$prefix -DCMAKE_POSITION_INDEPENDENT_CODE=ON"

dep_flags() {
	case "$1" in
		glog)        echo "-DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DWITH_GFLAGS=OFF" ;;
		leveldb)     echo "-DLEVELDB_BUILD_BENCHMARKS=OFF -DLEVELDB_BUILD_TESTS=OFF" ;;
		marisa-trie) echo "-DBUILD_TESTING=OFF -DENABLE_TOOLS=OFF" ;;
		opencc)      echo "-DBUILD_SHARED_LIBS=OFF -DOPENCC_BUILD_DATA=OFF -DOPENCC_BUILD_TOOLS=OFF" ;;
		yaml-cpp)    echo "-DYAML_CPP_BUILD_CONTRIB=OFF -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF" ;;
	esac
}

for dep in glog leveldb marisa-trie opencc yaml-cpp; do
	echo "== $dep ($abi) =="
	cmake -S "$rime_root/deps/$dep" -B "$build/$dep" $common $(dep_flags "$dep")
	cmake --build "$build/$dep" --target install -j"$(nproc 2>/dev/null || echo 4)"
done

echo "== librime ($abi) =="
cmake -S "$rime_root" -B "$build/rime" $common \
	-DBUILD_SHARED_LIBS=ON -DBUILD_STATIC=ON \
	-DBUILD_TEST=OFF -DBUILD_SAMPLE=OFF \
	-DCMAKE_PREFIX_PATH="$prefix"
cmake --build "$build/rime" --target rime -j"$(nproc 2>/dev/null || echo 4)"

lib="$build/rime/lib/librime.so"
[ -f "$lib" ] || { echo "no librime.so at $lib" >&2; exit 1; }
echo "OK $lib"
