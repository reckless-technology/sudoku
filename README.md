# sudoku

Jonny's Sudoku Solver: a Windows Forms program, written in C# in 2008, that solves a Sudoku by deduction - the
techniques a person uses, never a guess - either at once or a clue at a time.

## Using it

- Click a square and pick 1 to 9, or Clear, from the menu that opens there to enter the puzzle's givens.
- Clue places the next value the solver can deduce; Solve goes on until the board is full or nothing more can be
  deduced, and says which. Clear Board starts again.
- File > Save game and Open game keep a board in a `.sud` file: 81 bytes, one per square, row by row from the top left
  (byte `x + 9 * y`), 0 for an empty square. `Easy1.sud`, `Medium1.sud`, `Hard1.sud`, `Evil1.sud`, `Evil2.sud` and
  `X-wing1.sud` are puzzles in that form.
- The `.sdk` files are test puzzles as text, nine lines of nine characters with `.` for an empty square, each named
  for the technique it calls for (naked and hidden pairs, X-wing, swordfish, XY-wing, colouring, forcing chains,
  Nishio and others). The program does not open them.

## How it solves

`Board` holds the squares and `Square` each one's candidate values; `Solver`, behind the `ISudokuSolver` interface
(`Solver(3)` is the usual 9 x 9 board of 3 x 3 sectors), applies three strategies over every row, column and sector in
turn until one places a value or none makes progress:

- naked siblings: n squares of a row, column or sector whose candidates are only n values between them take those
  values, which are struck from the other squares there (n = 1 is a placed value's elimination);
- hidden siblings: n values whose only places in a row, column or sector are the same n squares take those squares,
  and every other candidate is struck from them (n = 1 is the single place for a value);
- line and sector intersection: a value whose candidates in a sector lie on one line is struck from the rest of that
  line, and one whose candidates on a line lie in one sector from the rest of that sector.

## Building

`Solver.sln` holds the one project, `Sudoku.csproj`, as Visual Studio 2010 left it: it targets the .NET Framework 2.0,
whose reference assemblies today's MSBuild needs the .NET Framework 3.5 SP1 for, and its ClickOnce settings sign the
manifests with `Sudoku_TemporaryKey.pfx`, which is not in the repository. With those in place:

```
msbuild Solver.sln -p:Configuration=Release      # bin/Release/Sudoku.exe
```

## History

The program began in the `software` repository, under `sudoku/`; it was extracted with its history by git-filter-repo.
The code, the project files and the puzzles are as they were there.
