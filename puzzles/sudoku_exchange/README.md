# Sudoku Exchange puzzles

A sample of the Sudoku Exchange "Puzzle Bank" (<https://github.com/grantm/sudoku-exchange-puzzle-bank>, commit
`d8c8ebaee0c08c412cfba96af1923dfa61c83317`): computer-generated puzzles, each with one solution (QQWing made them), rated
by Sukaku Explainer and sorted into four files by their rating. The data set is in the public domain (`LICENSE.txt`, the
Unlicense).

| File             | Rating      | Records here | Records in the bank |
| ---------------- | ----------- | ------------ | ------------------- |
| `easy.txt`       | below 1.5   | 250          | 100,000             |
| `medium.txt`     | below 2.5   | 500          | 352,643             |
| `hard.txt`       | below 5.0   | 1,000        | 321,592             |
| `diabolical.txt` | 5.0 or more | 500          | 119,681             |

Each file here is the first records of the bank's file of the same name, unchanged. The bank keeps its records in the
order of a hash of their digits, so the first are a sample of the whole. Each record is 100 bytes: 12 hex digits of
that hash, a space, the 81 squares row by row from the top left (0 for an empty square), a space, the rating
right-aligned in four characters (` 1.2`, `10.4`) and a line feed. `.gitattributes` keeps them byte for byte.

`tests/solver/rated_puzzle_test.cpp` solves every one by deduction and holds every step to its solution. Run over
the whole bank, the solver finishes all 893,916 puzzles with no step their solutions contradict - the easy ones by
singles alone, the medium ones by nothing past pointing; the hardest take up to about a second.
