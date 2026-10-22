# CLAUDE.md

Guidance for Claude Code in this repository. The README is the long-form reference: using it, how it solves, the
window, building, tests.

Reckless Sudoku: a Sudoku solved by deduction, a step at a time, C++20, its window on reckless-toolkit (a private
submodule at `external/reckless-toolkit`, added with its GUI on, its suites, its audio device layer and ASIO off). A
private repository, proprietary (`LICENSE`; `EULA.txt` for the releases' users). It began as a C# program in 2008; the
C# is in the history, with the solver's design and its puzzles carried over.

## Layout

- `src/solver/` (`sudoku_solver`, no toolkit): `Grid` (values and candidates), `Step` and `Technique`, the techniques in
  `techniques/` (each returns its first deduction without changing the board), `Solver` (simplest first, starting again
  after each step), `CountSolutions` (`brute_force.h`, the search that checks a puzzle), `puzzle_file` (`.sud` bytes and
  text), `Game` (the givens, the steps, the clashes, undo; every change to the givens rebuilds the board from them).
  The descriptions are what the window shows: change one and its tests change with it.
- `src/app/` (`sudoku_gui`, `RecklessSudoku`, `sudoku_render`): `Controller` (the window's state and every message, no
  toolkit), `BoardView` (draws a `BoardPicture`; clicks, the right-click digit menu, the keys), `SidePanel` (headline,
  message, plates, steps list), `MainView` (the series' header with the company mark over the board and the panel),
  `menus`, `AboutView`, `settings_file` (`reckless-technology/sudoku/settings.json`), `theme.h` (the Reckless dark
  look: cream ink, amber accent, Noto Serif, Monaspace Xenon for the pencil marks).
- `tests/solver/` (`sudoku_solver_test`): every puzzle in `puzzles/` solved with every step held to the solution, every
  technique called directly on every board on the way and held to it, built patterns for those no puzzle needs.
  `tests/app/` (`sudoku_app_test`): controller, menus, settings file, views painted and driven.
- `assets/icon/`: the icon at every size, drawn by `sudoku_render <out.png> icon <size>` and packed by
  `tools/make_icon.py` (Pillow); change the drawing (`render_main.cpp`'s `RenderIcon`), run the script, commit what it
  writes - never edit a render.
- The version is `MAJOR.MINOR.BUILD` from the nearest `vMAJOR.MINOR` tag (`cmake/sudoku_version.cmake`); 0.0.0 with no
  tag reachable.

## Rules

- File and directory names lower_snake_case (Jonny's rule for every repository); classes and functions PascalCase,
  members `snake_case_`, constants `kPascalCase`, booleans `is`/`has`/`can`/`may`/`do`/`does`, names spelled out;
  `/** */` on declarations with `@param`/`@return`, `///<` on members; British spelling in comments and documents,
  which describe the code as it is.
- `.clang-format` is pawnstar's; clang-format 22.1.8 only (the pre-commit hook and CI check it).
- Every commit authored and committed as `Jonny Reckless <jonny@recklesstechnology.com>`
  (`git config core.hooksPath .githooks` installs the hooks that check it). Work on a branch, open a pull request, merge
  when CI is green.
- CI runs on the organisation's self-hosted runners (the repository is private), reading the toolkit through the
  read-only deploy key whose private half is the secret `RECKLESS_TOOLKIT_DEPLOY_KEY`. A bump of the submodule points
  at a commit on the toolkit's `main`.
- Every new behaviour gets a test; the solver's soundness is held by the puzzle and technique suites, which must stay
  green: a step that strikes a solution's candidate is a bug, never a flaky test.
- No releases, tags or promotions without Jonny's word.
