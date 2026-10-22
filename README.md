# Reckless Sudoku

A Sudoku solved by deduction - the techniques a person uses, never a guess - a clue at a time or all at once, saying at
each step what it saw. C++20, its window drawn with Skia on reckless-toolkit, for Windows, macOS and Linux. It began in
2008 as a C# program; the C# is in the history.

## Using it

- **Entering a puzzle.** Click a square, or move to it with the arrow keys, and type 1 to 9; 0, Delete, Backspace or
  Space clears it. Right-click a square for the same choices as a menu. The digits entered are the givens, drawn in
  bold cream. Two givens that clash are washed red and named beside the board, and the solver waits until they are put
  right; givens that leave a square no digit, or a digit no place, are named the same way.
- **Clue** (F5) takes steps until one places a value; **Solve** (F6) goes on until the board is full or nothing more can
  be deduced. The solver's values are drawn in amber. Every step is listed beside the board in words - "Hidden pair
  {3,7} in row 4 (r4c2, r4c6) removes 1 from r4c2" - and choosing one lights it: the squares it reasoned from washed in
  amber, the value it placed in green, the candidates it struck in red.
- **Check** (F7) searches for the puzzle's solutions and says whether it has none, one or more than one. When Solve stops
  short it says why the same way: no solution, more than one, or one that needs a deeper chain than the solver follows.
- **Pencil marks** (View, Ctrl+P, or the panel's switch): each empty square shows the digits it may still take, in
  Monaspace Xenon.
- **Undo** (Ctrl+Z) takes back the last change - a given, a clue, a solve, an opened puzzle or a cleared board - one at a
  time. Changing a given clears the solver's values, which were deduced from the old givens; Clue or Solve finds them
  again.
- **Files** (File, Ctrl+O and Ctrl+S): a `.sud` file is 81 bytes, one per square row by row from the top left, 0 for an
  empty square; a `.sdk` file is nine lines of nine characters, `.` for an empty square. Open reads either, and any text
  file holding 81 squares; Save writes the givens. A puzzle's file named on the command line opens with the window.
- **The window's size** (View): 50 % to 150 % of its design size, drawn at exactly that scale; the size chosen and the
  pencil marks are kept in `reckless-technology/sudoku/settings.json` in the user's settings folder.

## How it solves

`src/solver` holds the solver, with no toolkit. A `Grid` holds each square's value and its candidates, the digits it may
still take; placing a value strikes it from the 20 squares that share a row, column or box with it. Each technique looks
at the grid and returns the first deduction it finds as a `Step` - what it saw, the values it places, the candidates it
strikes, the squares it reasoned from. The `Solver` tries the techniques simplest first, makes the first step found, and
starts again from the simplest:

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

`CountSolutions` is a search, not a technique: it counts a puzzle's solutions (none, one, more than one) for Check, and
the suites hold every step to the solution it finds. `Game` is the board a person works on - the givens, the solver's
steps, the clashes, the undo history; every change to the givens rebuilds the board from the givens alone, so nothing
deduced from a value outlives it. `puzzle_file` reads and writes the files.

The 68 puzzles in `puzzles/` are named for the technique each calls for. The solver finishes all of them. Puzzles built
to defeat human techniques - AI Escargot, Easter Monster, Arto Inkala's - need chains deeper than one trial followed by
singles, and the solver stops on them, saying the puzzle has one solution.

## The window

`src/app` is the window on reckless-toolkit's app shell, in the Reckless apps' look: the series' header with the company
mark, the name and the version; the dark window with cream ink and the amber accent; Noto Serif for the interface and
Monaspace Xenon for the pencil marks. `Controller` holds the window's state and words every message, written against no
toolkit; `BoardView` draws a `BoardPicture` at any size and takes the mouse and the keys; `SidePanel` holds the headline,
the message, the plates and the steps list; `MainView` lays them out under the header; `menus` builds the menu bar;
`AboutView` is Help > About; `settings_file` keeps the choices. `sudoku_render` draws the window in a chosen state to a
PNG, and the app's icon at any size (`tools/make_icon.py` packs it into `assets/icon/`).

## Building

C++20 with CMake 3.25 or later and Ninja; on Windows, in Visual Studio's developer environment. The window needs the
toolkit, a private submodule at `external/reckless-toolkit`:

```
git submodule update --init
cmake --preset release && cmake --build --preset release && ctest --preset release
build/release/src/app/RecklessSudoku            # the app (RecklessSudoku.exe on Windows, reckless-sudoku on Linux)
cmake --preset release -DSUDOKU_GUI=OFF         # the solver and its suites alone, with no submodule
```

The presets: `debug`, `release`, `release-universal` (macOS, Apple silicon and Intel in one binary), `sanitize`
(AddressSanitizer and UndefinedBehaviorSanitizer), `tsan` (ThreadSanitizer), `coverage-windows` (clang-cl). The version
is `MAJOR.MINOR.BUILD` from the nearest `vMAJOR.MINOR` tag and the commits since it (`cmake/sudoku_version.cmake`).

## Tests

- `sudoku_solver_test`: every puzzle solved by deduction with every step checked against the solution; every technique
  called on every puzzle as it is solved, whatever it finds held to the solution; patterns built for the techniques no
  puzzle here needs; the game, the files.
- `sudoku_app_test`: the controller and the menus, the settings file, and the views painted into images and driven by
  events.
- The render tool drawing every state and the icon, the app's launch (`app_smoke`), and the third-party notices held to
  the build's pins (`notices`).

## Licence

Proprietary (`LICENSE`); the releases' users are licensed under `EULA.txt`. The third-party parts the program carries
are in `THIRD-PARTY-NOTICES.txt`.
