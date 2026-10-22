#pragma once
/// @file seeing.h The squares that see a set of others, for the techniques whose conclusion is "what sees both ends
/// loses it": the wings and the XY-chain.

#include "solver/grid.h"
#include "solver/step.h"

#include <span>
#include <vector>

namespace sudoku::techniques
{

/**
 * @param grid The board.
 * @param digit A digit.
 * @param seen Squares.
 * @param also Squares passed over besides those seen.
 * @return The digit as a candidate in every empty square that sees all of `seen`, but for those of `seen` and `also`,
 * in order.
 */
std::vector<Candidate> Seeing(const Grid &grid, int digit, std::span<const int> seen, std::span<const int> also);

} // namespace sudoku::techniques
