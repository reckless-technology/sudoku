#!/usr/bin/env python3
"""Write THIRD-PARTY-NOTICES.txt: what Reckless Sudoku's executable contains from elsewhere, each part's licence as it
ships.

Every licence text is read from the source it governs, in the trees the build fetched (FETCHCONTENT_BASE_DIR, read from
the build tree's CMakeCache.txt, or the tree a FETCHCONTENT_SOURCE_DIR_<NAME> entry there names in its place) or the
toolkit's own assets, and every version from the toolkit's pins (external/reckless-toolkit/cmake/ToolkitPins.cmake),
the fetched headers or the fonts themselves, so the notices can only say what the build compiles. The parts are the
ones the executable holds, read off its link map: Skia and the libraries it bundles, which the window draws through -
Skia's own code with its colour management (skcms) and its PNG decoder, libpng and zlib, whose SIMD routines are the
Chromium authors', and on Linux FreeType - and the two typefaces it carries, Noto Serif for the interface and Monaspace
Xenon for the pencil marks - and the puzzle bank installed beside it, the Sudoku Exchange's, whose commit is read from
cmake/sudoku_puzzle_bank.cmake and whose licence from the repository's sample of it. Of the toolkit's other fetches none is in the executable: Monocypher (the licence check) and
the audio file decoders (dr_libs, stb) have nothing of theirs linked, the audio device layer (RtAudio, RtMidi) and the
ASIO SDK are not built (RECKLESS_BUILD_AUDIO is off), and GoogleTest is the suites' alone. The skia-pack carries the
licences of libpng, zlib and FreeType, read from it here; Skia's own and the Chromium source tree's, which the pack
lacks, are fetched by the toolkit beside it (external/reckless-toolkit/skia/CMakeLists.txt) and read from there. The
four platforms' packs carry the same files, so the notices are the same from any build tree; a licence file the pack
gains stops the script until it is read.

    python3 tools/third_party_notices.py <build-dir>           writes THIRD-PARTY-NOTICES.txt at the root
    python3 tools/third_party_notices.py <build-dir> --check   exits 1 if the committed file is not what it writes
                                                               (CTest notices)

Any build tree with the window on can write the notices or check them. Run it when the submodule moves, and read the
diff. Any Python 3 runs it, with nothing installed.
"""
import re
import struct
import sys
import textwrap
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TOOLKIT = ROOT / "external" / "reckless-toolkit"
WIDTH = 100

# Every licence-like file of the skia-pack release: the three the notices reproduce (libpng's, zlib's, FreeType's), the
# GPL FreeType also offers, those of libwebp and HarfBuzz, which nothing links, and those of ANGLE, SwiftShader and the
# Vulkan headers, whose headers alone the pack carries. A file outside the list is a licence the pack has gained, to be
# read before the notices are written again.
SKIA_PACK_LICENCE_FILES = {
    "third_party/externals/libpng/LICENSE", "third_party/externals/zlib/LICENSE",
    "third_party/externals/libwebp/COPYING", "third_party/externals/libwebp/PATENTS",
    "third_party/externals/harfbuzz/COPYING", "third_party/externals/freetype/docs/FTL.TXT",
    "third_party/externals/freetype/docs/GPLv2.TXT", "third_party/externals/angle2/LICENSE",
    "third_party/externals/swiftshader/LICENSE.txt", "include/third_party/vulkan/LICENSE",
}

APP = "Reckless Sudoku"


def read(path):
    return Path(path).read_text(encoding="utf-8", errors="strict").replace("\r\n", "\n")


def uncomment(block):
    """A licence quoted in a C or C++ comment, with the comment's markers taken off and its own layout kept: a run of //
    lines loses each line's // and the space after it; a /* */ block loses its two delimiters alone, since a line inside
    one that starts with an asterisk is the licence's own bullet. Then the common indent goes."""
    lines = [line.rstrip() for line in block.split("\n")]
    if all(line.lstrip().startswith("//") or not line.strip() for line in lines):
        lines = [re.sub(r"^\s*// ?", "", line) for line in lines]
    else:
        lines[0] = re.sub(r"^\s*/\*+ ?", "", lines[0])
        lines[-1] = re.sub(r"\s*\*+/$", "", lines[-1])
    return textwrap.dedent("\n".join(lines)).strip("\n")


def pins():
    """The toolkit's pins the notices name, from its pins file, which its CMakeLists.txt reads."""
    found = dict(re.findall(r'set\((RECKLESS_[A-Z0-9_]+)\s+"([^"]*)"', read(TOOLKIT / "cmake" / "ToolkitPins.cmake")))
    names = ("RECKLESS_SKIA_TAG", "RECKLESS_SKIA_COMMIT", "RECKLESS_CHROMIUM_COMMIT")
    missing = [n for n in names if n not in found]
    if missing:
        sys.exit(f"the toolkit's pins do not set {missing}")
    return {n: found[n] for n in names}


def fetched_trees(build):
    """A lookup of a fetch's source by its name: the tree a FETCHCONTENT_SOURCE_DIR_<NAME> entry of the cache names in
    the fetch's place, else <FETCHCONTENT_BASE_DIR>/<name>-src."""
    cache_file = Path(build) / "CMakeCache.txt"
    if not cache_file.is_file():
        sys.exit(f"{build} is no configured build tree (it has no CMakeCache.txt)")
    cache = read(cache_file)
    m = re.search(r"^FETCHCONTENT_BASE_DIR:PATH=(.*)$", cache, re.M)
    base = Path(m.group(1).strip()) if m else Path(build) / "_deps"

    def src(name):
        given = re.search(rf"^FETCHCONTENT_SOURCE_DIR_{name.upper()}:PATH=(.*)$", cache, re.M)
        if given and given.group(1).strip():
            return Path(given.group(1).strip())
        return base / f"{name}-src"

    return src


def skia_pack_dir(src):
    """The skia-pack release's root: whichever platform's package the build fetched (each carries the same headers and
    licence files)."""
    for name in ("skia_macos_arm64", "skia_macos_x64", "skia_windows_x64", "skia_linux_x64"):
        if (src(name) / "third_party" / "externals").is_dir():
            return src(name)
    sys.exit("the notices carry the licences of what the window links of the skia-pack, read from the pack, which this "
             "build tree did not fetch: a tree with the window on (SUDOKU_GUI)")


def font_revision(path):
    """A font's revision as its head table records it (a 16.16 fixed-point number), to three decimals."""
    data = Path(path).read_bytes()
    count = struct.unpack(">H", data[4:6])[0]
    for i in range(count):
        tag, _, offset, _ = struct.unpack(">4sIII", data[12 + 16 * i: 28 + 16 * i])
        if tag == b"head":
            return f"{struct.unpack('>i', data[offset + 4: offset + 8])[0] / 65536:.3f}"
    sys.exit(f"{path} has no head table")


def family_revision(name, faces):
    """The one revision a family's faces share, or a stop where they differ."""
    versions = {font_revision(face) for face in faces}
    if len(versions) != 1:
        sys.exit(f"{name}'s faces are of different revisions ({sorted(versions)}): name each")
    return versions.pop()


def wrap(paragraph):
    words, lines, line = paragraph.split(), [], ""
    for word in words:
        if len(line) + len(word) + (line != "") > WIDTH:
            lines.append(line)
            line = ""
        line += ("" if line == "" else " ") + word
    lines.append(line)
    return "\n".join(lines)


def prose(*paragraphs):
    return "\n\n".join(wrap(p) for p in paragraphs)


def section(number, title, where, *parts):
    rule = "=" * WIDTH
    body = "\n\n".join(p.strip("\n") for p in parts if p)
    return f"{rule}\n{number}. {title}\nUsed in: {where}\n{rule}\n\n{body}\n"


def build(build_dir):
    p = pins()
    src = fetched_trees(build_dir)

    fonts = TOOLKIT / "assets" / "fonts"
    noto_version = family_revision("Noto Serif", [fonts / f"NotoSerif-{face}.ttf" for face in ("Regular", "Bold")])
    monaspace_version = family_revision(
        "Monaspace Xenon", [fonts / "monaspace" / f"MonaspaceXenon-{face}.otf" for face in ("Regular", "Bold")])
    ofl_noto = read(fonts / "OFL.txt").strip("\n")
    ofl_monaspace = read(fonts / "monaspace" / "OFL.txt").strip("\n")

    pack = skia_pack_dir(src)
    externals = pack / "third_party" / "externals"
    licence_files = {f.relative_to(pack).as_posix() for f in pack.rglob("*")
                     if f.is_file() and re.fullmatch(r"(licen[cs]e|copying|patents|ftl|gplv?\d*)(\.txt|\.md)?",
                                                     f.name, re.I)}
    if licence_files != SKIA_PACK_LICENCE_FILES:
        sys.exit(f"the skia-pack release's licence files are not the ones the notices read - new: "
                 f"{sorted(licence_files - SKIA_PACK_LICENCE_FILES)}, gone: "
                 f"{sorted(SKIA_PACK_LICENCE_FILES - licence_files)}: read them")
    for header in (pack / "include" / "core" / "SkCanvas.h", pack / "modules" / "skcms" / "skcms.h"):
        if "Use of this source code is governed by a BSD-style license" not in read(header):
            sys.exit(f"{header.name} no longer gives its licence as Skia's BSD-style licence: read it")
    for header in ("adler32_simd.h", "crc32_simd.h", "cpu_features.h", "slide_hash_simd.h"):
        if "found in the Chromium source repository LICENSE file" not in read(externals / "zlib" / header):
            sys.exit(f"zlib's {header} no longer points at the Chromium source repository's LICENSE: read it")
    for fetched in ("skia_licence", "chromium_licence"):
        if not (src(fetched) / "LICENSE").is_file():
            sys.exit(f"the toolkit fetches {fetched} beside the skia-pack, and this build tree has the pack without "
                     "it: configure it again")
    skia_licence = read(src("skia_licence") / "LICENSE").strip("\n")
    chromium_licence = uncomment(read(src("chromium_licence") / "LICENSE"))
    skia_milestone = re.search(r"#define SK_MILESTONE (\d+)",
                               read(pack / "include" / "core" / "SkMilestone.h")).group(1)
    skia_png_version = re.search(r'#define PNG_LIBPNG_VER_STRING "([^"]+)"',
                                 read(externals / "libpng" / "png.h")).group(1)
    skia_zlib_version = re.search(r'#define ZLIB_VERSION "([^"]+)"', read(externals / "zlib" / "zlib.h")).group(1)
    freetype_header = read(externals / "freetype" / "include" / "freetype" / "freetype.h")
    freetype_version = ".".join(re.search(rf"#define FREETYPE_{part}\s+(\d+)", freetype_header).group(1)
                                for part in ("MAJOR", "MINOR", "PATCH"))
    freetype_year = re.search(r"Copyright \(C\) \d{4}-(\d{4}) by", freetype_header).group(1)
    ftl = read(externals / "freetype" / "docs" / "FTL.TXT")
    freetype_credit = " ".join(re.search(r'"""\s*(Portions of this software.*?)\s*"""', ftl, re.S).group(1).split())
    freetype_credit = freetype_credit.replace("<year>", freetype_year)

    bank_commit = re.search(r"set\(SUDOKU_PUZZLE_BANK_COMMIT ([0-9a-f]{40})\)",
                            read(ROOT / "cmake" / "sudoku_puzzle_bank.cmake")).group(1)
    bank_licence = read(ROOT / "puzzles" / "sudoku_exchange" / "LICENSE.txt").strip("\n")

    # The sections in their order, each numbered by its place.
    keys = ["noto", "monaspace", "skia", "png_skia", "zlib_skia", "freetype", "bank"]
    n = {key: i + 1 for i, key in enumerate(keys)}
    sections = {
        "noto": (f"Noto Serif {noto_version}", APP, prose(
            "Reckless Sudoku carries Noto Serif's regular and bold faces inside its executable and draws its interface "
            "in them."), ofl_noto),
        "monaspace": (f"Monaspace Xenon {monaspace_version}", APP, prose(
            "Reckless Sudoku carries Monaspace Xenon's regular and bold faces inside its executable and sets the "
            "pencil marks in them."), ofl_monaspace),
        "skia": (f"Skia m{skia_milestone}, skia-pack {p['RECKLESS_SKIA_TAG']}", APP, prose(
            f"Reckless Sudoku draws its window through Skia m{skia_milestone}, linked from the prebuilt static "
            f"libraries of the skia-pack release {p['RECKLESS_SKIA_TAG']}, built from Skia's commit "
            f"{p['RECKLESS_SKIA_COMMIT']}. It holds Skia's own code - its raster renderer, its colour management "
            "(skcms) and its PNG decoder - and, of the libraries the release bundles, libpng and zlib (sections "
            f"{n['png_skia']} and {n['zlib_skia']}), and on Linux FreeType (section {n['freetype']}), which draws the "
            "text there; it declines every other image decoder, so no other bundled library is linked.",
            "Skia's licence, the LICENSE file of its source tree at that commit, which skcms, a part of the same tree, "
            "shares:"), skia_licence),
        "png_skia": (f"libpng {skia_png_version}, in Skia", APP, read(externals / "libpng" / "LICENSE")),
        "zlib_skia": (f"zlib {skia_zlib_version}, in Skia", APP, read(externals / "zlib" / "LICENSE"), prose(
            "Its SIMD routines (adler32_simd, crc32_simd and crc_folding, inffast_chunk, cpu_features and "
            "slide_hash_simd) are the Chromium authors', under the LICENSE file of the Chromium source tree at "
            f"revision {p['RECKLESS_CHROMIUM_COMMIT']}, the revision the zlib commit Skia's DEPS names was copied "
            "from:"), chromium_licence),
        "freetype": (f"FreeType {freetype_version}, in Skia", f"{APP} on Linux", prose(
            "This software is based in part on the work of the FreeType Team. FreeType is used under the FreeType "
            "License, the first of the two licences it offers.", freetype_credit), ftl),
        "bank": (f"Sudoku Exchange puzzle bank, commit {bank_commit[:7]}", f"{APP}'s puzzles", prose(
            "Reckless Sudoku installs the Sudoku Exchange \"Puzzle Bank\" (https://github.com/grantm/sudoku-exchange-"
            f"puzzle-bank, commit {bank_commit}) beside its executable, unchanged, and offers new puzzles from it: "
            "puzzles QQWing generated, rated by Sukaku Explainer. Its authors put it in the public domain:"),
            bank_licence),
    }

    rows = []
    for key in keys:
        title, used = sections[key][0], sections[key][1]
        rows.append(f"  {n[key]:>2}. {title.split(',')[0]:<44} {used}")
    head = ("THIRD-PARTY NOTICES\n\n" + prose(
        "Reckless Sudoku is copyright (C) 2008-2026 Jonny Reckless and licensed under the GNU General Public "
        "License, version 3 or later (LICENSE). Its executable, and "
        "the puzzles installed beside it, contain the third-party software and data below, each part used under its "
        "own licence, which this file reproduces wherever the licence asks for it. Nothing in the GPL limits any right these licences give you.") +
        "\n\nWhere each part is used\n\n" + "\n".join(rows) + "\n")
    parts = [head] + [section(n[key], *sections[key]) for key in keys]
    return "\n\n".join(s.rstrip("\n") for s in parts) + "\n"


def main():
    if len(sys.argv) < 2 or sys.argv[1].startswith("--"):
        sys.exit(__doc__)
    text = build(sys.argv[1])
    out = ROOT / "THIRD-PARTY-NOTICES.txt"
    if "--check" in sys.argv:
        current = out.read_text(encoding="utf-8").replace("\r\n", "\n") if out.exists() else ""
        if current != text:
            sys.exit(f"{out.name} is not what {Path(__file__).name} writes from {sys.argv[1]}: run it again")
        print(f"{out.name} is current")
        return
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
    print(f"wrote {out} ({len(text.splitlines())} lines)")


if __name__ == "__main__":
    main()
