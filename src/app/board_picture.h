#pragma once
/// @file board_picture.h What the board shows: each square's value and pencil marks, the square chosen, the clashes,
/// and the step lit - the squares it reasoned from, what it read there, the values it placed, the candidates it struck.

#include "solver/digits.h"
#include "solver/grid.h"
#include "solver/step.h"

#include <algorithm>
#include <array>
#include <vector>

namespace sudoku::app
{

/** How a pencil mark is drawn. */
enum class MarkLook
{
    kHidden, ///< Not drawn.
    kPlain,  ///< A candidate, quiet.
    kReason, ///< A candidate the lit step read, ringed.
    kStruck, ///< A candidate the lit step struck, red.
};

/** What the board shows, made by the controller and drawn by the board's view. */
struct BoardPicture
{
    Values                                 values{};     ///< Each square's value, 0 while empty.
    std::array<bool, geometry::kCellCount> is_given{};   ///< Whether each value was entered.
    std::array<int, geometry::kCellCount>  candidates{}; ///< Each square's pencil marks (digits.h).
    int                    selected{-1}; ///< The square chosen, which the keys fill; -1 for none, as at launch.
    bool                   shows_pencil_marks{true}; ///< Whether empty squares show their marks.
    std::vector<int>       clashes;                  ///< The squares whose givens clash.
    std::vector<int>       pattern;                  ///< The lit step's squares.
    std::vector<int>       placed;                   ///< The squares the lit step placed values in.
    std::vector<Candidate> struck;                   ///< The candidates the lit step struck.
    std::vector<Candidate> reasons; ///< What the lit step read: pencil marks and values (Step::reasons).

    /**
     * How a digit is drawn in a square's pencil marks. The lit step's marks - those it read and those it struck - are
     * drawn even with the pencil marks hidden: they are what the person asked to see.
     * @param cell A square.
     * @param digit A digit.
     * @return Hidden in a square with a value, struck, read, a plain candidate while the marks show, or hidden.
     */
    MarkLook LookOf(int cell, int digit) const
    {
        const auto index = static_cast<std::size_t>(cell);
        if (values[index] != 0)
        {
            return MarkLook::kHidden;
        }
        if (std::find(struck.begin(), struck.end(), Candidate{cell, digit}) != struck.end())
        {
            return MarkLook::kStruck;
        }
        if (std::find(reasons.begin(), reasons.end(), Candidate{cell, digit}) != reasons.end())
        {
            return MarkLook::kReason;
        }
        return shows_pencil_marks && digits::Contains(candidates[index], digit) ? MarkLook::kPlain : MarkLook::kHidden;
    }

    /**
     * @param cell A square.
     * @return Whether its value is one the lit step read - a value that rules a single's other choices out - and not
     * one it placed, ringed.
     */
    bool IsReasonValue(int cell) const
    {
        const int value = values[static_cast<std::size_t>(cell)];
        return value != 0 && std::find(placed.begin(), placed.end(), cell) == placed.end() &&
               std::find(reasons.begin(), reasons.end(), Candidate{cell, value}) != reasons.end();
    }

    /**
     * @param other Another picture.
     * @return Whether the two show the same.
     */
    bool operator==(const BoardPicture &other) const = default;
};

} // namespace sudoku::app
