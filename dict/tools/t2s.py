"""Shared plumbing for the dictionary generators in this directory.

Both generators re-encode text with opencc, so both need the same three things:
the protected-character set, a placeholder pass around it, and a single opencc
invocation over the whole file. The conversion rule itself stays in each script.
"""

import os
import pathlib
import subprocess
import sys
import tempfile

HERE = pathlib.Path(__file__).resolve().parent
DICT_DIR = HERE.parent

# PUA block: opencc has no entry for these, so they survive the converter
# untouched and can be swapped back afterwards.
_PLACEHOLDERS = ["\ue000", "\ue001", "\ue002", "\ue003", "\ue004", "\ue005"]


def protected_chars():
    chars = []
    for line in (HERE / "protected_chars.txt").read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if line and not line.startswith("#"):
            chars.append(line)
    if len(chars) > len(_PLACEHOLDERS):
        sys.exit(f"protected_chars.txt lists {len(chars)} chars, only {len(_PLACEHOLDERS)} placeholders")
    return chars


def opencc_cli():
    cli = os.environ.get("OPENCC_CLI")
    if not cli:
        sys.exit("OPENCC_CLI is not set -- run this through dict/tools/Makefile (make toolchain)")
    if not os.path.exists(cli):
        sys.exit(f"{cli} not found; build it with `make toolchain`")
    return cli


def convert(text):
    """Run `text` through opencc with the character-level config.

    Whole-file, one process: opencc is line-preserving on this input and a
    per-word subprocess call would cost 400k process spawns.
    """
    paths = [str(DICT_DIR / "opencc"), os.environ.get("OPENCC_CONFIG_DIR", "")]
    paths = [p for p in paths if p]
    cmd = [opencc_cli(), "--config", str(HERE / "t2s_char.json")]
    for p in paths:
        cmd += ["--path", p]
    # Temp files rather than stdin/stdout: the CLI only guarantees the -i/-o
    # paths, and 400k lines over a pipe is a second thing to get wrong.
    with tempfile.TemporaryDirectory(prefix="dictgen-") as tmp:
        src, dst = os.path.join(tmp, "in"), os.path.join(tmp, "out")
        with open(src, "w", encoding="utf-8") as fh:
            fh.write(text)
        cmd += ["--input", src, "--output", dst]
        proc = subprocess.run(cmd, capture_output=True)
        if proc.returncode:
            sys.exit(f"opencc failed: {proc.stderr.decode('utf-8', 'replace')[:400]}")
        return pathlib.Path(dst).read_text(encoding="utf-8")


def simplify(text):
    """Character-level traditional -> simplified, keeping protected chars."""
    chars = protected_chars()
    # opencc has no entry for the PUA placeholders, so a protected char survives
    # the converter only if its mask appears the same number of times in the
    # output. Count is the invariant: fewer masks = opencc converted/lost that
    # char (a bare `ph in out` test would be true on the success path too).
    counts = {ch: text.count(ch) for ch in chars}
    masked = text
    for ph, ch in zip(_PLACEHOLDERS, chars):
        masked = masked.replace(ch, ph)
    out = convert(masked)
    for ph, ch in zip(_PLACEHOLDERS, chars):
        if out.count(ph) != counts[ch]:
            sys.exit(
                f"placeholder {ph!r} count {out.count(ph)} != source count "
                f"{counts[ch]} -- protected char {ch!r} was converted/lost by opencc"
            )
    for ph, ch in zip(_PLACEHOLDERS, chars):
        out = out.replace(ph, ch)
    return out


def pin(name):
    """Read a key out of sources.lock."""
    for line in (HERE / "sources.lock").read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if line and not line.startswith("#") and "=" in line:
            key, _, value = (p.strip() for p in line.partition("="))
            if key == name:
                return value
    sys.exit(f"sources.lock has no {name}")
