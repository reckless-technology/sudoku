#include "solver/grid.h"

#include "solver/digits.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace sudoku
{

Grid::Grid()
{
    candidates_.fill(digits::kAll);
}

Grid Grid::FromValues(std::span<const int> values)
{
    if (values.size() != static_cast<std::size_t>(geometry::kCellCount))
    {
        throw std::invalid_argument("a board has " + std::to_string(geometry::kCellCount) + " squares, not " +
                                    std::to_string(values.size()));
    }
    for (int value : values)
    {
        if (value < 0 || value > 9)
        {
            throw std::invalid_argument("a square's value is 0 to 9, not " + std::to_string(value));
        }
    }
    Grid grid;
    for (int cell = 0; cell < geometry::kCellCount; cell++)
    {
        int value = values[static_cast<std::size_t>(cell)];
        if (value != 0)
        {
            grid.Place(cell, value);
        }
    }
    return grid;
}

bool Grid::IsComplete() const
{
    return std::find(values_.begin(), values_.end(), 0) == values_.end();
}

void Grid::Place(int cell, int digit)
{
    values_[static_cast<std::size_t>(cell)]     = digit;
    candidates_[static_cast<std::size_t>(cell)] = digits::Bit(digit);
    for (int peer : geometry::Peers(cell))
    {
        if (values_[static_cast<std::size_t>(peer)] == 0)
        {
            candidates_[static_cast<std::size_t>(peer)] &= ~digits::Bit(digit);
        }
    }
}

bool Grid::Eliminate(int cell, int digit)
{
    if (Value(cell) != 0 || !digits::Contains(Candidates(cell), digit))
    {
        return false;
    }
    candidates_[static_cast<std::size_t>(cell)] &= ~digits::Bit(digit);
    return true;
}

bool Grid::HasPlaced(const Unit &unit, int digit) const
{
    return std::any_of(unit.cells.begin(), unit.cells.end(), [&](int cell) { return Value(cell) == digit; });
}

std::vector<int> Grid::PlacesFor(const Unit &unit, int digit) const
{
    std::vector<int> places;
    for (int cell : unit.cells)
    {
        if (Value(cell) == 0 && digits::Contains(Candidates(cell), digit))
        {
            places.push_back(cell);
        }
    }
    return places;
}

std::optional<std::string> Grid::FindContradiction() const
{
    for (const Unit &unit : geometry::Units())
    {
        // The values in the order they first appear in the unit; the first that appears again is named.
        for (std::size_t i = 0; i < unit.cells.size(); i++)
        {
            int value = Value(unit.cells[i]);
            if (value == 0 || std::any_of(unit.cells.begin(), unit.cells.begin() + static_cast<std::ptrdiff_t>(i),
                                          [&](int cell) { return Value(cell) == value; }))
            {
                continue;
            }
            std::vector<int> twice;
            std::copy_if(unit.cells.begin() + static_cast<std::ptrdiff_t>(i), unit.cells.end(),
                         std::back_inserter(twice), [&](int cell) { return Value(cell) == value; });
            if (twice.size() > 1)
            {
                return std::to_string(value) + " is twice in " + unit.Name() + " (" + geometry::Names(twice) + ")";
            }
        }
    }
    for (int cell = 0; cell < geometry::kCellCount; cell++)
    {
        if (Value(cell) == 0 && Candidates(cell) == 0)
        {
            return geometry::Name(cell) + " has no digit left";
        }
    }
    for (const Unit &unit : geometry::Units())
    {
        for (int digit = 1; digit <= 9; digit++)
        {
            if (!HasPlaced(unit, digit) && PlacesFor(unit, digit).empty())
            {
                return std::to_string(digit) + " has no place left in " + unit.Name();
            }
        }
    }
    return std::nullopt;
}

std::vector<int> Grid::Clashes(std::span<const int> values)
{
    std::vector<int> clashes;
    for (int cell = 0; cell < geometry::kCellCount; cell++)
    {
        int                        value = values[static_cast<std::size_t>(cell)];
        const std::array<int, 20> &peers = geometry::Peers(cell);
        if (value != 0 && std::any_of(peers.begin(), peers.end(),
                                      [&](int peer) { return values[static_cast<std::size_t>(peer)] == value; }))
        {
            clashes.push_back(cell);
        }
    }
    return clashes;
}

} // namespace sudoku
