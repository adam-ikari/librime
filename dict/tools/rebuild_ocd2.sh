#!/bin/sh
# Regenerate the opencc data this dictionary ships, from the vendored opencc in
# ../../deps/opencc. Everything here is derived from opencc's own tables, so the
# pin in sources.lock is what makes it reproducible.
#
# The engine never runs this: dict/opencc/ ships as data (see ../README.md).
set -eu

out=${1:?usage: rebuild_ocd2.sh <output-dir>}
build=${OPENCC_BUILD:?OPENCC_BUILD must point at the opencc build tree}
src=${OPENCC_SRC:?OPENCC_SRC must point at the vendored opencc source}
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
dict="$script_dir/../opencc"

mkdir -p "$out"

# .ocd2 is the binary form opencc loads at runtime; it is built from opencc's
# text tables by opencc_dict during opencc's own data step.
for f in TSCharacters TSPhrases HKVariants TWVariants; do
	[ -f "$build/data/$f.ocd2" ] || { echo "missing $build/data/$f.ocd2 -- run make toolchain first" >&2; exit 1; }
	cp "$build/data/$f.ocd2" "$out/"
done

# The configs librime reads by name. t2s.json is upstream's; t2hk/t2tw are
# upstream's too and only exist here because librime resolves them out of
# shared_data_dir/opencc, not out of opencc's install prefix.
for f in t2s t2hk t2tw; do
	[ -f "$src/data/config/$f.json" ] || { echo "missing $src/data/config/$f.json" >&2; exit 1; }
	cp "$src/data/config/$f.json" "$out/"
done

# t2s_full.json and the variants* tables are ours (see ../LICENSE); regeneration
# would overwrite hand-curated data, so they are only checked, never written.
for f in t2s_full.json variants.txt variants_ext.txt variants_jp.txt; do
	cp "$dict/$f" "$out/$f"
done

echo "opencc data -> $out"
