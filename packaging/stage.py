#!/usr/bin/env python3
"""What a release ships, staged from a release build tree into a folder of its own: the program as the build made it,
its end-user licence and the third-party notices. Everything after it - the archive, the installers, the signing - reads
the stage, never the build tree.

    python3 -I packaging/stage.py <build-dir> <dist-dir> linux|macos|windows

    windows  RecklessSudoku.exe, EULA.txt, THIRD-PARTY-NOTICES.txt
    linux    reckless-sudoku, EULA.txt, THIRD-PARTY-NOTICES.txt - the archive as it is, and what the Debian package lays
             out
    macos    RecklessSudoku.app (copied with ditto, its signature as the build made it: ad hoc, or a release's Developer
             ID after packaging/macos/sign.sh), EULA.txt, THIRD-PARTY-NOTICES.txt

The folder is emptied first; a binary the build did not make stops it before anything is copied. The licence is the
repository's EULA.txt, the one every installer shows; the repository's own LICENSE, which the program's About card
carries, is not shipped as a file. Any Python 3 runs it, with nothing installed.
"""
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

BINARIES = {
    "linux": "src/app/reckless-sudoku",
    "macos": "src/app/RecklessSudoku.app",
    "windows": "src/app/RecklessSudoku.exe",
}
DOCUMENTS = ("EULA.txt", "THIRD-PARTY-NOTICES.txt")


def main():
    arguments = [a for a in sys.argv[1:] if not a.startswith("--")]
    if len(arguments) != 3 or arguments[2] not in BINARIES:
        sys.exit(__doc__)
    build, dist, platform_name = Path(arguments[0]), Path(arguments[1]), arguments[2]
    source = build / BINARIES[platform_name]
    if not source.exists():
        sys.exit(f"stage.py: {source} is not there: build the release tree first")
    if dist.exists():
        shutil.rmtree(dist)
    dist.mkdir(parents=True)
    if platform_name == "macos":
        subprocess.run(["ditto", str(source), str(dist / source.name)], check=True)
        subprocess.run(["codesign", "--verify", "--deep", "--strict", str(dist / source.name)], check=True)
    else:
        shutil.copy2(source, dist / source.name)
    for document in DOCUMENTS:
        shutil.copyfile(ROOT / document, dist / document)
    print(f"staged into {dist}:")
    for path in sorted(dist.rglob("*")):
        relative = path.relative_to(dist).as_posix()
        if path.is_file() and (".app/" not in relative or "/Contents/MacOS/" in relative):
            print(f"  {relative}  {path.stat().st_size} bytes")


if __name__ == "__main__":
    main()
