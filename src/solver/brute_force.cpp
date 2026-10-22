#include "solver/brute_force.h"

#include "solver/digits.h"

#include <optional>

namespace sudoku
{

namespace
{

/**
 * Searches depth first, always at the empty square with the fewest candidates.
 * @param grid The board so far.
 * @param limit How many solutions are enough.
 * @param first Where the first solution found goes; left as it is once it holds one.
 * @return The solutions found, up to the limit.
 */
int Search(const Grid &grid, int limit, std::optional<Values> &first)
{
    int best   = -1;
    int fewest = 10;
    for (int cell = 0; cell < geometry::kCellCount; cell++)
    {
        if (!grid.IsEmpty(cell))
        {
            continue;
        }
        int count = digits::Count(grid.Candidates(cell));
        if (count == 0)
        {
            return 0;
        }
        if (count < fewest)
        {
            fewest = count;
            best   = cell;
        }
    }
    if (best < 0)
    {
        if (!first)
        {
            first = grid.values();
        }
        return 1;
    }
    int total = 0;
    for (int digit : digits::Each(grid.Candidates(best)))
    {
        Grid trial = grid;
        trial.Place(best, digit);
        total += Search(trial, limit - total, first);
        if (total >= limit)
        {
            break;
        }
    }
    return total;
}

} // namespace

SolutionCount CountSolutions(std::span<const int> values, Values *solution)
{
    Grid grid = Grid::FromValues(values);
    if (grid.FindContradiction())
    {
        return SolutionCount::kNone;
    }
    std::optional<Values> first;
    int                   found = Search(grid, 2, first);
    if (first && solution != nullptr)
    {
        *solution = *first;
    }
    return found == 0 ? SolutionCount::kNone : found == 1 ? SolutionCount::kOne : SolutionCount::kMany;
}

} // namespace sudoku
