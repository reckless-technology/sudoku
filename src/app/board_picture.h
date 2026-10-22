#pragma once
/// @file board_picture.h What the board shows: each square's value and pencil marks, the square chosen, the clashes,
/// and the step lit - the squares it reasoned from, the values it placed, the candidates it struck.

#include "solver/grid.h"
#include "solver/step.h"

#include <array>
#include <vector>

namespace sudoku::app
{

/** What the board shows, made by the controller and drawn by the board's view. */
struct BoardPicture
{
    Values                                 values{};                 ///< Each square's value, 0 while empty.
    std::array<bool, geometry::kCellCount> is_given{};               ///< Whether each value was entered.
    std::array<int, geometry::kCellCount>  candidates{};             ///< Each square's pencil marks (digits.h).
    int                                    selected{40};             ///< The square chosen, which the keys fill.
    bool                                   shows_pencil_marks{true}; ///< Whether empty squares show their marks.
    std::vector<int>                       clashes;                  ///< The squares whose givens clash.
    std::vector<int>                       pattern;                  ///< The lit step's squares.
    std::vector<int>                       placed;                   ///< The squares the lit step placed values in.
    std::vector<Candidate>                 struck;                   ///< The candidates the lit step struck.

    /**
     * @param other Another picture.
     * @return Whether the two show the same.
     */
    bool operator==(const BoardPicture &other) const = default;
};

} // namespace sudoku::app
