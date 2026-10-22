#include "solver/digits.h"
#include "solver/techniques/techniques.h"

#include <vector>

namespace sudoku::techniques
{

std::optional<Step> NakedSingle(const Grid &grid)
{
    for (int cell = 0; cell < geometry::kCellCount; cell++)
    {
        if (grid.IsEmpty(cell) && digits::Count(grid.Candidates(cell)) == 1)
        {
            int digit = digits::First(grid.Candidates(cell));
            return Step{Technique::kNakedSingle,
                        "Naked single: " + geometry::Name(cell) + " can only be " + std::to_string(digit),
                        {Candidate{cell, digit}},
                        {},
                        {cell}};
        }
    }
    return std::nullopt;
}

std::optional<Step> HiddenSingle(const Grid &grid)
{
    for (const Unit &unit : geometry::Units())
    {
        for (int digit = 1; digit <= 9; digit++)
        {
            if (grid.HasPlaced(unit, digit))
            {
                continue;
            }
            std::vector<int> places = grid.PlacesFor(unit, digit);
            if (places.size() == 1)
            {
                return Step{Technique::kHiddenSingle,
                            "Hidden single: " + std::to_string(digit) + " has one place in " + unit.Name() + ", " +
                                geometry::Name(places[0]),
                            {Candidate{places[0], digit}},
                            {},
                            std::vector<int>(unit.cells.begin(), unit.cells.end())};
            }
        }
    }
    return std::nullopt;
}

} // namespace sudoku::techniques
