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

`src/core` holds the solver, with no window. A `Grid` holds each square's value and its candidates, the digits it may
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

- `src/core` (`Sudoku.Core`): the grid, the techniques (`techniques/`), the solver, the search, the files, the game.
- `src/app` (`Sudoku`): the window - `MainForm`, and `BoardView`, which draws the board at any size and DPI and takes
  the mouse and the keys.
- `tests` (`Sudoku.Tests`, xUnit): every puzzle solved by deduction with every step checked against the solution;
  every technique called on every puzzle as it is solved, whatever it finds held to the solution; the game, the files.
- `puzzles`: the puzzles, as `.sdk` text and a few `.sud` boards.

## Building

With the .NET 10 SDK, on Windows:

```
dotnet build sudoku.slnx -c Release      # src/app/bin/Release/net10.0-windows/Sudoku.exe
dotnet test sudoku.slnx -c Release
```

Every warning is an error. `.github/workflows/build.yml` builds and tests every push to `main` and every pull request
on the organisation's self-hosted Windows runner, and keeps `Sudoku.exe` as the run's artifact.

## History

The program began in the `software` repository, under `sudoku/`; it was extracted with its history by git-filter-repo.
Its first solver knew naked and hidden subsets and the line and box intersection, which finish 23 of the 68 puzzles
here.
