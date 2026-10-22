#include "solver/geometry.h"

#include <algorithm>

namespace sudoku
{

namespace
{

/** The board's units and each square's peers, built once. */
struct Shape
{
    std::array<Unit, 9>                                   rows{};    ///< The rows, top to bottom.
    std::array<Unit, 9>                                   columns{}; ///< The columns, left to right.
    std::array<Unit, 9>                                   boxes{};   ///< The boxes, left to right and top to bottom.
    std::array<Unit, 27>                                  units{};   ///< The rows, then the columns, then the boxes.
    std::array<std::array<int, 20>, geometry::kCellCount> peers{};   ///< Each square's peers, in order.

    /** Builds the units and the peers. */
    Shape()
    {
        for (int index = 0; index < 9; index++)
        {
            Unit &row    = rows[static_cast<std::size_t>(index)];
            Unit &column = columns[static_cast<std::size_t>(index)];
            Unit &box    = boxes[static_cast<std::size_t>(index)];
            row.kind     = UnitKind::kRow;
            column.kind  = UnitKind::kColumn;
            box.kind     = UnitKind::kBox;
            row.index = column.index = box.index = index;
            for (int i = 0; i < 9; i++)
            {
                row.cells[static_cast<std::size_t>(i)]    = geometry::Cell(index, i);
                column.cells[static_cast<std::size_t>(i)] = geometry::Cell(i, index);
                box.cells[static_cast<std::size_t>(i)] = geometry::Cell(index / 3 * 3 + i / 3, index % 3 * 3 + i % 3);
            }
            units[static_cast<std::size_t>(index)]      = row;
            units[static_cast<std::size_t>(index + 9)]  = column;
            units[static_cast<std::size_t>(index + 18)] = box;
        }
        for (int cell = 0; cell < geometry::kCellCount; cell++)
        {
            std::size_t count = 0;
            for (int other = 0; other < geometry::kCellCount; other++)
            {
                if (geometry::Sees(cell, other))
                {
                    peers[static_cast<std::size_t>(cell)][count++] = other;
                }
            }
        }
    }
};

/** @return The board's shape, built on first use. */
const Shape &TheShape()
{
    static const Shape shape;
    return shape;
}

} // namespace

std::string Unit::Name() const
{
    const char *kind_name = kind == UnitKind::kRow ? "row" : kind == UnitKind::kColumn ? "column" : "box";
    return std::string(kind_name) + " " + std::to_string(index + 1);
}

bool Unit::Contains(int cell) const noexcept
{
    return std::find(cells.begin(), cells.end(), cell) != cells.end();
}

namespace geometry
{

std::string Name(int cell)
{
    return "r" + std::to_string(Row(cell) + 1) + "c" + std::to_string(Column(cell) + 1);
}

std::string Names(std::span<const int> cells)
{
    std::string text;
    for (int cell : cells)
    {
        if (!text.empty())
        {
            text += ", ";
        }
        text += Name(cell);
    }
    return text;
}

const std::array<Unit, 9> &Rows()
{
    return TheShape().rows;
}

const std::array<Unit, 9> &Columns()
{
    return TheShape().columns;
}

const std::array<Unit, 9> &Boxes()
{
    return TheShape().boxes;
}

const std::array<Unit, 27> &Units()
{
    return TheShape().units;
}

const std::array<int, 20> &Peers(int cell)
{
    return TheShape().peers[static_cast<std::size_t>(cell)];
}

} // namespace geometry

} // namespace sudoku
