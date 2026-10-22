# sudoku

Jonny's Sudoku Solver: a Windows program that solves a Sudoku by deduction - the techniques a person uses, never a
guess - a clue at a time or all at once, saying at each step what it saw. Written in C# in 2008; rebuilt in 2026 on
.NET 10 with the techniques a hard puzzle calls for.

## Using it

- **Entering a puzzle.** Click a square, or move to it with the arrow keys, and type 1 to 9; 0, Delete, Backspace or
  Space clears it. Right-click a square for the same choices as a menu. The digits you enter are the givens, drawn in
  bold. Two givens that clash are shaded red and named on the status line, and the solver waits until they are put
  right.
- **Clue** (F5) takes steps until one places a value; **Solve** (F6) goes on until the board is full or nothing more
  can be deduced. The solver's values are drawn in blue. Every step is listed beside the board, in words - "Hidden
  pair {3,7} in row 4 (r4c2, r4c6) removes 1 from r4c2" - and choosing one lights it: the squares it reasoned from in
  blue, the value it placed in green, the candidates it struck in red.
- **Check** (F7) searches for the puzzle's solutions and says whether it has none, one or more than one. When Solve
  stops short it says why the same way: no solution, more than one, or one that needs a deeper chain than the solver
  follows.
- **Pencil marks** (View, Ctrl+P): each empty square shows the digits it may still take.
- **Undo** (Ctrl+Z) takes back the last change - a given, a clue, a solve, an opened puzzle or a cleared board - one at a
  time. Changing a given clears the solver's values, which were deduced from the old givens; Clue or Solve finds them
  again.
- **Files** (File, Ctrl+O and Ctrl+S): a `.sud` file is 81 bytes, one per square row by row from the top left, 0 for an
  empty square; a `.sdk` file is nine lines of nine characters, `.` for an empty square. Open reads either, and any
  text file holding 81 squares; Save writes the givens. A puzzle's file named on the command line opens with the window.

## How it solves

`src/solver` holds the solver, with no window. A `Grid` holds each square's value and its candidates, the digits it may
still take; placing a value strikes it from the 20 squares that share a row, column or box with it. Each technique
looks at the grid and returns the first deduction it finds as a `Step` - what it saw, the values it places, the
candidates it strikes, the squares it reasoned from. The `Solver` tries the techniques simplest first, makes the first
step found, and starts again from the simplest:

1. Naked single: a square with one candidate. Hidden single: a digit with one place in a row, column or box.
2. Naked and hidden pairs, then pointing and claiming (a digit's places in a box on one line, or on a line in one box),
   then naked and hidden triples and quads. A naked subset's squares need not each hold all its digits: {1,2}, {2,3},
   {1,3} is a naked triple.
3. X-wing, swordfish and jellyfish: a digit's places on two, three or four rows confined to as many columns, or the
   other way round.
4. XY-wing and XYZ-wing; simple colouring (a digit's chains of strong links, coloured alternately); XY-chains.
5. Nishio (a digit placed and its own singles followed until a unit has no place for it), forcing chains (every
   candidate of a square leading, by singles, to the same conclusion) and contradiction chains (a candidate leading, by
   singles, to a contradiction).

`BruteForce` is a search, not a technique: it counts a puzzle's solutions (none, one, more than one) for Check, and the
suites hold every step to the solution it finds. `Game` is the board a person works on - the givens, the solver's
steps, the clashes, the undo history - which the window draws; every change to the givens rebuilds the board from the
givens alone, so nothing deduced from a value outlives it. `PuzzleFile` reads and writes the files.

The 68 puzzles in `puzzles/` are named for the technique each calls for. The solver finishes all of them. Puzzles
built to defeat human techniques - AI Escargot, Easter Monster, Arto Inkala's - need chains deeper than one trial
followed by singles, and the solver stops on them, saying the puzzle has one solution.

## Layout

- `src/solver` (`sudoku_solver`): the grid, the techniques (`techniques/`), the solver, the search, the files, the game,
  on nothing but the standard library.
- `src/app` (`sudoku_gui`, `RecklessSudoku`, `sudoku_render`): the window on reckless-toolkit - the controller, the
  board, the side panel, the menus, the About card, the settings kept in `reckless-technology/sudoku/settings.json` in
  the user's settings folder.
- `tests` (GoogleTest): `sudoku_solver_test` - every puzzle solved by deduction with every step checked against the
  solution, every technique called on every puzzle as it is solved, the game, the files - and `sudoku_app_test`, the
  controller, the settings file and the views painted and driven; the render tool's runs and the window's launch.
- `puzzles`: the puzzles, as `.sdk` text and a few `.sud` boards.
- `packaging`, `.github`: the installers and CI, below.

## Building

The window is built on reckless-toolkit, a private git submodule at `external/reckless-toolkit`; `-DSUDOKU_GUI=OFF`
builds the solver and its suite alone, with no submodule.

On Windows, in Visual Studio's developer environment (`VsDevCmd.bat -arch=x64 -host_arch=x64 -no_logo`), with CMake and
Ninja:

```
git submodule update --init                      # the toolkit, external/reckless-toolkit (private)
cmake --preset release && cmake --build --preset release && ctest --preset release
build/release/src/app/RecklessSudoku.exe
```

On Linux (Debian, Ubuntu or Mint, with gcc) the packages are
`build-essential cmake ninja-build pkg-config git libfreetype-dev libfontconfig1-dev libx11-dev`, the same presets
build `build/release/src/app/reckless-sudoku`, and the window's launch (`app_smoke`, label `window`) needs a display,
which `xvfb-run -a ctest --preset release -L window` gives it where there is none. The `sanitize` preset builds
everything under AddressSanitizer and UndefinedBehaviorSanitizer, the `tsan` preset under ThreadSanitizer (gcc or
clang, Linux or macOS), each with a build and a test preset of the same name; neither suppresses anything. On macOS,
with Xcode's command-line tools and Homebrew's CMake and Ninja, the `release-universal` preset builds one binary for
Apple silicon and Intel, `build/release-universal/src/app/RecklessSudoku.app`; every binary runs on macOS 13.3 and
later.

### The tools

- `build/release/src/app/sudoku_render <out.png> <state>` draws the window in a state (`empty`, `puzzle`, `clue`,
  `solved`, `stuck`, `clash`, `no-marks`, `about`) to a PNG, and `sudoku_render <out.png> icon <size>` the icon. The
  suites run it for every state.
- `powershell -ExecutionPolicy Bypass -File tools\coverage-windows.ps1` measures the line coverage of `src/` (clang-cl
  and LLVM's source-based coverage, the `coverage-windows` preset) over every suite, the window's launch and the render
  tool's runs, and fails under 95 %; `src/app/main.cpp`, which opens the window and the event loop, is left out.
- `python tools/make_icon.py build/release/src/app/sudoku_render.exe` draws the icon at every size into `assets/icon/`
  (Pillow); the build takes them from there and needs no Python.
- `python3 tools/third_party_notices.py build/release` writes `THIRD-PARTY-NOTICES.txt` from the build tree's fetches
  and the toolkit's pins (any Python 3, nothing installed); the CTest `notices` runs it with `--check` and fails when the
  committed file is not what it writes. Run it when the submodule moves, and read the diff.
- C++ is formatted with clang-format 22.1.8 (`.clang-format`), which the pre-commit hook and CI check. In a fresh clone:
  `git config core.hooksPath .githooks`.

### The version

Every binary reports `MAJOR.MINOR.BUILD` (`cmake/sudoku_version.cmake`): `MAJOR.MINOR` is the nearest tag of the form
`vMAJOR.MINOR` reachable from the commit built, and `BUILD` the number of commits since it. A checkout with no such tag
builds as 0.0.0, with a warning.

## Packaging

`packaging/` turns a release build tree into what is shipped, the same way on a pull request's run and a release's:

- `packaging/stage.py <build-dir> <dist-dir> windows|linux|macos` stages the program (`RecklessSudoku.exe`,
  `reckless-sudoku` or `RecklessSudoku.app`), `EULA.txt` and `THIRD-PARTY-NOTICES.txt` into a folder of their own;
  everything after it reads the stage.
- Windows: `packaging/windows/make_installer.ps1` compiles `sudoku.iss` with Inno Setup 6.7.3 (fetched and held to its
  hash with `-Fetch`) into `reckless-sudoku-<version>-win64-setup.exe`. It shows the end-user licence, then installs for
  the current user by default, under the user's Programs folder in `Reckless Technology\Sudoku`, or, at the dialog's
  choice, for every user under `Program Files\Reckless Technology\Sudoku`, beside the organisation's other products,
  with a Start-menu entry in the shared `Reckless Technology` group. Its uninstaller takes away what it installed and
  nothing else. `test_installer.ps1` installs a test build of it under a folder of its own, checks every file, launches
  the installed program for its launch check, installs it again over itself and uninstalls it, with two sibling
  products' folders beside it left as they were.
- macOS: `packaging/macos/make_installer.sh` makes `reckless-sudoku-<version>-macos.pkg`, for macOS 13.3 and later on
  either kind of Mac: the end-user licence shown, then `RecklessSudoku.app` and `Uninstall Reckless Sudoku.app` in
  `/Applications/Reckless Technology`, the licence and the notices in `/Library/Documentation/Reckless Sudoku`. The
  uninstaller asks, then removes those (each bundle only when its identifier is the program's) and the package's
  receipts, removes the shared folder only when it is empty, and removes the person's settings only if asked.
  `test_installer.sh` holds the package to the stage and runs the uninstaller's script on a stand-in for `/` beside
  another product's app. `sign.sh` signs the staged app for a release.
- Linux: `packaging/linux/make_deb.sh` makes the Debian package `reckless-sudoku-<version>-linux-amd64.deb` - the
  program at `/opt/reckless-technology/bin/reckless-sudoku`, a menu entry, the icon, the licence, the notices and a copyright
  file under `/usr/share/doc/reckless-sudoku` - its Depends read off the binary; `test_deb.sh` holds it to the stage.
  `make_tarball.sh` makes the portable `reckless-sudoku-<version>-linux-x64.tar.xz`, the stage in one folder, and holds
  it to the stage.

## CI

`.github/workflows/build.yml`, on the organisation's self-hosted runners (the repository is private; the short jobs on
the runners labelled `light`), on every push to `main`, every pull request and by hand:

- Windows with MSVC builds everything, runs every suite including the window's launch and the notices' check, makes the
  installer unsigned and puts it through `test_installer.ps1`, and keeps `RecklessSudoku.exe` and the installer as the
  run's artifacts.
- Linux with gcc builds the program as it ships, runs every suite and the window's launch under `xvfb-run`, makes the
  Debian package and the tarball, and keeps them.
- macOS builds the `release-universal` preset, holds every binary to both slices (`lipo`) and the floor of 13.3
  (`vtool`), runs every suite (and the solver's suite again under Rosetta 2), makes the installer package unsigned, and
  keeps it.
- The sanitizer jobs build everything and run every suite under AddressSanitizer and UndefinedBehaviorSanitizer, and
  again under ThreadSanitizer.
- The coverage job builds with gcc instrumented, runs every suite but the window's launch, and fails under 95 % of the
  lines of `src/` (gcovr 8.3), printing each directory's figure; another job checks the formatting.
- `CI` is green only when every job above is: the one check a ruleset requires.

The artifacts are kept for 30 days. A newer push to a pull request cancels its older run; every commit on `main` keeps
its own. The toolkit submodule is read through its read-only deploy key, the secret `RECKLESS_TOOLKIT_DEPLOY_KEY`.

## Releases

`.github/workflows/release.yml` runs on a pushed tag `vMAJOR.MINOR`, or by hand on `main`
(`gh workflow run release.yml --ref main`, with `-f dry_run=true` to make nothing but the run's artifacts). It works out
the version as the build does, refuses a commit `main` does not hold, opens a draft GitHub release `vMAJOR.MINOR.BUILD`,
builds from nothing on the three platforms, runs every suite, packages as above and uploads the four files - the
Windows installer, the macOS package, the Debian package and the tarball - onto the draft, then checks they are all
there. The release stays a private draft: nothing in the workflow publishes it, makes its tag or copies it elsewhere. A
draft of the same version from an earlier run is replaced; a published one stops the run.

Signing uses the organisation's secrets where the repository has them, and is left out with a warning where it has not:
on Windows the installer and its uninstaller are signed through SSL.com's eSigner (the toolkit's
`tools/esigner-sign.ps1`, when `ESIGNER_USERNAME` is set); on macOS the app is signed with the Developer ID Application
identity and the hardened runtime, the package with the Developer ID Installer identity, then notarised and stapled (the
toolkit's `tools/sign-mac.sh`, when `MACOS_DEVELOPER_ID_APPLICATION_P12` is set).

## Licence

Reckless Sudoku is proprietary (`LICENSE`); the program is licensed to the people who use it under `EULA.txt`, which
every installer shows and carries. The third-party parts the executable contains (Skia with libpng, zlib and, on Linux,
FreeType; the Noto Serif and Monaspace Xenon typefaces) are listed with their licences in `THIRD-PARTY-NOTICES.txt`,
which every package installs and the executable carries.

## History

The program began in the `software` repository, under `sudoku/`; it was extracted with its history by git-filter-repo.
Its first solver knew naked and hidden subsets and the line and box intersection, which finish 23 of the 68 puzzles
here.
