#pragma once
/// @file geometry.h The board's shape: 81 squares numbered row by row from the top left (square column + 9 * row), its
/// 27 units - the rows, the columns and the boxes - and the 20 squares each square shares a unit with.

#include <array>
#include <span>
#include <string>

namespace sudoku
{

/** What kind of unit a row, a column or a box is. */
enum class UnitKind
{
    kRow,    ///< A row, left to right.
    kColumn, ///< A column, top to bottom.
    kBox,    ///< A 3 x 3 box, numbered left to right and top to bottom.
};

/** Nine squares that must hold each digit once: a row, a column or a box. */
struct Unit
{
    UnitKind           kind{UnitKind::kRow}; ///< A row, a column or a box.
    int                index{0};             ///< Which one, from 0.
    std::array<int, 9> cells{};              ///< Its nine squares, in reading order.

    /** @return Its name: "row 4", "column 7", "box 5". */
    std::string Name() const;

    /**
     * @param cell A square.
     * @return Whether the unit holds it.
     */
    bool Contains(int cell) const noexcept;
};

namespace geometry
{

inline constexpr int kCellCount = 81; ///< The squares on the board.

/**
 * @param cell A square.
 * @return Its row, from 0.
 */
constexpr int Row(int cell) noexcept
{
    return cell / 9;
}

/**
 * @param cell A square.
 * @return Its column, from 0.
 */
constexpr int Column(int cell) noexcept
{
    return cell % 9;
}

/**
 * @param cell A square.
 * @return Its box, from 0.
 */
constexpr int Box(int cell) noexcept
{
    return cell / 27 * 3 + cell % 9 / 3;
}

/**
 * @param row A row, from 0.
 * @param column A column, from 0.
 * @return The square where they cross.
 */
constexpr int Cell(int row, int column) noexcept
{
    return row * 9 + column;
}

/**
 * @param a A square.
 * @param b Another square.
 * @return Whether the two are different squares that share a unit, so cannot hold the same digit.
 */
constexpr bool Sees(int a, int b) noexcept
{
    return a != b && (Row(a) == Row(b) || Column(a) == Column(b) || Box(a) == Box(b));
}

/**
 * @param cell A square.
 * @return Its name, row and column from 1: "r4c7".
 */
std::string Name(int cell);

/**
 * @param cells Squares.
 * @return Their names, joined: "r4c2, r4c6".
 */
std::string Names(std::span<const int> cells);

/** @return The rows, top to bottom. */
const std::array<Unit, 9> &Rows();

/** @return The columns, left to right. */
const std::array<Unit, 9> &Columns();

/** @return The boxes, left to right and top to bottom. */
const std::array<Unit, 9> &Boxes();

/** @return Every unit: the rows, then the columns, then the boxes. */
const std::array<Unit, 27> &Units();

/**
 * @param cell A square.
 * @return The 20 squares that share a unit with it, in order.
 */
const std::array<int, 20> &Peers(int cell);

} // namespace geometry

} // namespace sudoku
