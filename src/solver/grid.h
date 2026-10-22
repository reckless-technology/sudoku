#pragma once
/// @file grid.h A board being solved: each square's value and the digits it may still take.

#include "solver/geometry.h"

#include <array>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace sudoku
{

/** 81 values, square column + 9 * row, 0 for an empty square. */
using Values = std::array<int, geometry::kCellCount>;

/**
 * A board being solved: each square's value, 0 while it is empty, and the digits it may still take - its candidates.
 * Placing a digit strikes it from the candidates of the 20 squares that share a unit with the square. A value: copy it
 * to change one board without changing another.
 */
class Grid
{
  public:
    /** An empty board: every square may take every digit. */
    Grid();

    /**
     * A board with these values placed. Values that clash are placed all the same, so the board holds the contradiction
     * for FindContradiction to name.
     * @param values 81 values, 0 for an empty square.
     * @return The board.
     * @throws std::invalid_argument For other than 81 values, or one outside 0 to 9.
     */
    static Grid FromValues(std::span<const int> values);

    /**
     * @param cell A square.
     * @return Its value, 1 to 9, or 0 while it is empty.
     */
    int Value(int cell) const
    {
        return values_[static_cast<std::size_t>(cell)];
    }

    /**
     * @param cell A square.
     * @return The digits it may take (digits.h): its value's alone once it has one.
     */
    int Candidates(int cell) const
    {
        return candidates_[static_cast<std::size_t>(cell)];
    }

    /**
     * @param cell A square.
     * @return Whether it has no value yet.
     */
    bool IsEmpty(int cell) const
    {
        return Value(cell) == 0;
    }

    /** @return Whether every square has a value. */
    bool IsComplete() const;

    /** @return The values, 0 for an empty square. */
    const Values &values() const noexcept
    {
        return values_;
    }

    /**
     * Gives a square its value, and strikes the digit from the candidates of the empty squares around it.
     * @param cell A square.
     * @param digit Its value, 1 to 9.
     */
    void Place(int cell, int digit);

    /**
     * Strikes a candidate from an empty square.
     * @param cell A square.
     * @param digit A digit.
     * @return Whether it was a candidate there, and is gone.
     */
    bool Eliminate(int cell, int digit);

    /**
     * @param unit A unit.
     * @param digit A digit.
     * @return Whether a square of the unit has the digit as its value.
     */
    bool HasPlaced(const Unit &unit, int digit) const;

    /**
     * @param unit A unit.
     * @param digit A digit.
     * @return The unit's empty squares that may still take the digit, in order.
     */
    std::vector<int> PlacesFor(const Unit &unit, int digit) const;

    /**
     * The first thing that makes the board impossible, looked for in this order: a digit twice in a unit (the units in
     * Units' order), an empty square with no candidate, a digit with no place left in a unit.
     * @return What it is, in words - "5 is twice in row 1 (r1c1, r1c9)", "r1c9 has no digit left", "7 has no place left
     * in box 3"; none for none.
     */
    std::optional<std::string> FindContradiction() const;

    /**
     * @param values 81 values, 0 for an empty square.
     * @return The squares whose value another square in one of its units also has, in order.
     */
    static std::vector<int> Clashes(std::span<const int> values);

    /**
     * @param other Another board.
     * @return Whether the two have the same values and the same candidates.
     */
    bool operator==(const Grid &other) const = default;

  private:
    Values                                values_{};     ///< Each square's value, 0 while it is empty.
    std::array<int, geometry::kCellCount> candidates_{}; ///< Each square's candidates.
};

} // namespace sudoku
