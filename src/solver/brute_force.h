#pragma once
/// @file brute_force.h Search by trial: not how the solver solves, but how a puzzle is checked - whether it has a
/// solution, only one, and what that solution is.

#include "solver/grid.h"

#include <span>

namespace sudoku
{

/** How many solutions a puzzle has, as far as it matters. */
enum class SolutionCount
{
    kNone, ///< No way to complete the board.
    kOne,  ///< Exactly one: a proper puzzle.
    kMany, ///< More than one: the givens are too few to decide it.
};

/**
 * Counts a puzzle's solutions, depth first from the empty square with the fewest candidates, stopping at two.
 * @param values 81 values, 0 for an empty square.
 * @param solution Where to put the first solution found, untouched when there is none; null for nowhere.
 * @return None, one or many.
 * @throws std::invalid_argument For other than 81 values, or one outside 0 to 9.
 */
SolutionCount CountSolutions(std::span<const int> values, Values *solution = nullptr);

} // namespace sudoku
