#include "solver/digits.h"
#include "solver/techniques/techniques.h"

#include <algorithm>
#include <vector>

namespace sudoku::techniques
{

namespace
{

/**
 * @param cells Squares, at least one.
 * @return The row all the squares share, where they share one, then the column all of them share, where they share
 * one.
 */
std::vector<const Unit *> LinesThrough(const std::vector<int> &cells)
{
    std::vector<const Unit *> lines;
    int                       row    = geometry::Row(cells[0]);
    int                       column = geometry::Column(cells[0]);
    if (std::all_of(cells.begin(), cells.end(), [&](int cell) { return geometry::Row(cell) == row; }))
    {
        lines.push_back(&geometry::Rows()[static_cast<std::size_t>(row)]);
    }
    if (std::all_of(cells.begin(), cells.end(), [&](int cell) { return geometry::Column(cell) == column; }))
    {
        lines.push_back(&geometry::Columns()[static_cast<std::size_t>(column)]);
    }
    return lines;
}

} // namespace

std::optional<Step> Pointing(const Grid &grid)
{
    for (const Unit &box : geometry::Boxes())
    {
        for (int digit = 1; digit <= 9; digit++)
        {
            std::vector<int> places = grid.PlacesFor(box, digit);
            if (places.size() < 2 || grid.HasPlaced(box, digit))
            {
                continue;
            }
            for (const Unit *line : LinesThrough(places))
            {
                std::vector<Candidate> eliminations;
                for (int cell : line->cells)
                {
                    if (geometry::Box(cell) != box.index && grid.IsEmpty(cell) &&
                        digits::Contains(grid.Candidates(cell), digit))
                    {
                        eliminations.push_back(Candidate{cell, digit});
                    }
                }
                if (!eliminations.empty())
                {
                    std::string description = "Pointing: " + std::to_string(digit) + " in " + box.Name() +
                                              " lies only in " + line->Name() + " (" + geometry::Names(places) +
                                              "), so the rest of " + line->Name() + " " + Removes(eliminations);
                    return Step{Technique::kPointing, std::move(description), {}, std::move(eliminations), places};
                }
            }
        }
    }
    return std::nullopt;
}

std::optional<Step> Claiming(const Grid &grid)
{
    std::vector<const Unit *> lines;
    for (const Unit &row : geometry::Rows())
    {
        lines.push_back(&row);
    }
    for (const Unit &column : geometry::Columns())
    {
        lines.push_back(&column);
    }
    for (const Unit *line : lines)
    {
        for (int digit = 1; digit <= 9; digit++)
        {
            std::vector<int> places = grid.PlacesFor(*line, digit);
            if (places.size() < 2 || grid.HasPlaced(*line, digit) ||
                !std::all_of(places.begin(), places.end(),
                             [&](int cell) { return geometry::Box(cell) == geometry::Box(places[0]); }))
            {
                continue;
            }
            const Unit            &box = geometry::Boxes()[static_cast<std::size_t>(geometry::Box(places[0]))];
            std::vector<Candidate> eliminations;
            for (int cell : box.cells)
            {
                if (!line->Contains(cell) && grid.IsEmpty(cell) && digits::Contains(grid.Candidates(cell), digit))
                {
                    eliminations.push_back(Candidate{cell, digit});
                }
            }
            if (!eliminations.empty())
            {
                std::string description = "Claiming: " + std::to_string(digit) + " on " + line->Name() +
                                          " lies only in " + box.Name() + " (" + geometry::Names(places) +
                                          "), so the rest of " + box.Name() + " " + Removes(eliminations);
                return Step{Technique::kClaiming, std::move(description), {}, std::move(eliminations), places};
            }
        }
    }
    return std::nullopt;
}

std::optional<Step> Intersections(const Grid &grid)
{
    if (std::optional<Step> step = Pointing(grid))
    {
        return step;
    }
    return Claiming(grid);
}

} // namespace sudoku::techniques
