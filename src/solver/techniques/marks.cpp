#include "solver/techniques/marks.h"

#include "solver/digits.h"

namespace sudoku::techniques
{

std::vector<Candidate> MarksIn(const Grid &grid, std::span<const int> cells, int set)
{
    std::vector<Candidate> marks;
    for (int cell : cells)
    {
        if (!grid.IsEmpty(cell))
        {
            continue;
        }
        for (int digit : digits::Each(grid.Candidates(cell) & set))
        {
            marks.push_back(Candidate{cell, digit});
        }
    }
    return marks;
}

std::optional<Candidate> RuledOutBy(const Grid &grid, int cell, int digit)
{
    for (int peer : geometry::Peers(cell))
    {
        if (grid.Value(peer) == digit)
        {
            return Candidate{peer, digit};
        }
    }
    return std::nullopt;
}

} // namespace sudoku::techniques
