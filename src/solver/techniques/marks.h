#pragma once
/// @file marks.h What a step reasoned from, for the board to ring: the pencil marks it read in the squares it looked
/// at, and for a single the placed values that rule the other choices out.

#include "solver/grid.h"
#include "solver/step.h"

#include <optional>
#include <span>
#include <vector>

namespace sudoku::techniques
{

/**
 * @param grid The board.
 * @param cells Squares.
 * @param set Digits (digits.h).
 * @return Each of the digits that is a candidate of each empty square of `cells`, in the order of `cells`, each
 * square's by digit.
 */
std::vector<Candidate> MarksIn(const Grid &grid, std::span<const int> cells, int set);

/**
 * @param grid The board.
 * @param cell A square.
 * @param digit A digit.
 * @return The value `digit` placed in a square that shares a unit with `cell`, the first of its peers: what rules the
 * digit out there; none where none does (an earlier step struck it).
 */
std::optional<Candidate> RuledOutBy(const Grid &grid, int cell, int digit);

} // namespace sudoku::techniques
