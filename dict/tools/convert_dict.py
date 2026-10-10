#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Regenerate dict/luna_pinyin.dict.yaml from its traditional base.

The base is the traditional snapshot committed in this repo at the SHA named by
`luna-pinyin-base` in sources.lock -- not upstream rime-luna-pinyin's current
head, which has since gained per-reading frequency weights and entries this
dictionary deliberately does not carry (Japanese national variants, the bopomofo
index). Reproducing the product means reproducing that curation, so the base is
read from git history rather than re-derived from upstream.

Contract:
  * the YAML header and its comments stay untouched -- they are prose, and
    converting them would silently traditionalize->simplify our own notes.
  * character-level t2s only, with protected characters kept.
  * drop the weight column: candidate order comes from essay.txt
    (use_preset_vocabulary), and a second, sparser frequency source in the word
    list only fights it.
  * deduplicate by (word, syllables), keeping the first reading line -- one word
    with several readings stays several lines, because typing either reading has
    to reach the word.
  * apply pinyin_fixes.txt on top of the converted base: the base is an
    upstream traditional snapshot, and a handful of its syllable strings are
    simply wrong for simplified Chinese (蚌埠市 bang fu shi). They cannot be
    fixed at the source -- the source is a pinned git object -- so the
    corrections live in a side table the generator reads every run.
  * order by the whole line.
"""

import argparse
import pathlib
import subprocess
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import t2s

REPO_ROOT = t2s.HERE.parent.parent


def base_from_git():
    sha = t2s.pin("luna-pinyin-base")
    proc = subprocess.run(
        ["git", "-C", str(REPO_ROOT), "show", f"{sha}:dict/luna_pinyin.dict.yaml"],
        capture_output=True, text=True)
    if proc.returncode:
        sys.exit(
            f"cannot read base {sha} from git history:\n{proc.stderr.strip()}\n"
            "This repo was probably cloned with --depth. Fetch the object first:\n"
            f"  git fetch --depth=1 origin {sha} && git cat-file -e {sha}^{{commit}}")
    return proc.stdout


def load_fixes():
    """Read tools/pinyin_fixes.txt into (fix, del, add) directives.

    fix {word: syllables}       -- replace every generated reading of the word
    del {word: {syllables}}     -- drop these specific readings
    add {word: {syllables}}     -- add readings, keep the rest of the word's
    """
    path = pathlib.Path(__file__).resolve().parent / "pinyin_fixes.txt"
    fixes, dels, adds = {}, {}, {}
    for lineno, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        fields = line.split("\t")
        if len(fields) != 3:
            sys.exit(f"pinyin_fixes.txt:{lineno}: expected 3 tab-separated fields: {line!r}")
        cmd, word, syllables = fields
        syllables = " ".join(syllables.split())
        if not syllables:
            sys.exit(f"pinyin_fixes.txt:{lineno}: no syllables: {line!r}")
        if cmd == "fix":
            fixes[word] = syllables
        elif cmd == "del":
            dels.setdefault(word, set()).add(syllables)
        elif cmd == "add":
            adds.setdefault(word, set()).add(syllables)
        else:
            sys.exit(f"pinyin_fixes.txt:{lineno}: unknown directive {cmd!r}")
    return fixes, dels, adds


def apply_fixes(pairs):
    """Apply pinyin_fixes.txt to converted (word, syllables) pairs."""
    fixes, dels, adds = load_fixes()
    words_present = {w for w, _ in pairs}
    dropped = 0
    for word, syllable_set in dels.items():
        have = {s for w, s in pairs if w == word}
        for s in sorted(syllable_set - have):
            print(f"  del {word} -> {s}  (not present, no-op)")
        for s in sorted(syllable_set & have):
            dropped += 1
            print(f"  del {word} -> {s}")
    kept = [(w, s) for w, s in pairs
            if w not in fixes and s not in dels.get(w, ())]
    for word, syllables in fixes.items():
        print(f"  fix {word} -> {syllables}"
              + ("" if word in words_present else "  (new: not in base)"))
        kept.append((word, syllables))
    for word, syllable_set in sorted(adds.items()):
        have = {s for w, s in kept if w == word}
        for s in sorted(syllable_set - have):
            print(f"  add {word} -> {s}")
            kept.append((word, s))
    return kept


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--base", help="traditional luna_pinyin.dict.yaml; read from git if omitted")
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    if args.base:
        source = pathlib.Path(args.base).read_text(encoding="utf-8")
    else:
        source = base_from_git()

    lines = source.splitlines()
    try:
        sep = lines.index("...")
    except ValueError:
        sys.exit("base has no '...' section marker -- wrong file?")
    header, body = lines[: sep + 1], lines[sep + 1:]

    entries = [l for l in body if l.strip() and not l.lstrip().startswith("#")]
    converted = t2s.simplify("\n".join(entries)).splitlines()

    seen, kept = set(), []
    for line in converted:
        fields = line.split("\t")
        if len(fields) < 2:
            sys.exit(f"entry without a reading: {line!r}")
        word = fields[0]
        syllables = " ".join(fields[1].split())
        if (word, syllables) in seen:
            continue
        seen.add((word, syllables))
        kept.append((word, syllables))

    kept = apply_fixes(kept)

    kept.sort()
    out = pathlib.Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text("\n".join(header + [f"{w}\t{s}" for w, s in kept]) + "\n",
                   encoding="utf-8")
    print(f"{out}: {len(kept)} entries (from {len(entries)} base lines)")


if __name__ == "__main__":
    main()
