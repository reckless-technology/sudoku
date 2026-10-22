#include "solver/digits.h"
#include "solver/techniques/marks.h"
#include "solver/techniques/techniques.h"

#include <algorithm>
#include <vector>

namespace sudoku::techniques
{

namespace
{

/**
 * Adds a reason, once.
 * @param reasons The reasons.
 * @param reason One more; none for none.
 */
void AddReason(std::vector<Candidate> &reasons, std::optional<Candidate> reason)
{
    if (reason && std::find(reasons.begin(), reasons.end(), *reason) == reasons.end())
    {
        reasons.push_back(*reason);
    }
}

/**
 * @param grid The board.
 * @param digit A digit.
 * @param cells Squares without it as a candidate.
 * @return Few values `digit` that between them see every square of `cells` a value sees: each in turn the one that
 * sees the most squares the others leave, the first in order on a tie.
 */
std::vector<Candidate> ValuesShutting(const Grid &grid, int digit, std::vector<int> cells)
{
    std::vector<int> holders;
    for (int cell = 0; cell < geometry::kCellCount; cell++)
    {
        if (grid.Value(cell) == digit)
        {
            holders.push_back(cell);
        }
    }
    std::vector<Candidate> values;
    while (true)
    {
        int best      = -1;
        int best_seen = 0;
        for (int holder : holders)
        {
            const auto seen =
                std::count_if(cells.begin(), cells.end(), [&](int cell) { return geometry::Sees(holder, cell); });
            if (seen > best_seen)
            {
                best      = holder;
                best_seen = static_cast<int>(seen);
            }
        }
        if (best < 0)
        {
            return values;
        }
        values.push_back(Candidate{best, digit});
        std::erase_if(cells, [&](int cell) { return geometry::Sees(best, cell); });
    }
}

} // namespace

std::optional<Step> NakedSingle(const Grid &grid)
{
    for (int cell = 0; cell < geometry::kCellCount; cell++)
    {
        if (grid.IsEmpty(cell) && digits::Count(grid.Candidates(cell)) == 1)
        {
            int digit = digits::First(grid.Candidates(cell));
            // The candidate left, and a value that sees the square for each digit it cannot be.
            std::vector<Candidate> reasons{Candidate{cell, digit}};
            for (int other = 1; other <= 9; other++)
            {
                if (other != digit)
                {
                    AddReason(reasons, RuledOutBy(grid, cell, other));
                }
            }
            return Step{Technique::kNakedSingle,
                        "Naked single: " + geometry::Name(cell) + " can only be " + std::to_string(digit),
                        {Candidate{cell, digit}},
                        {},
                        {cell},
                        std::move(reasons)};
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
                // The one place, and the fewest values of the digit that shut the unit's other empty squares.
                std::vector<int> others;
                for (int cell : unit.cells)
                {
                    if (cell != places[0] && grid.IsEmpty(cell))
                    {
                        others.push_back(cell);
                    }
                }
                std::vector<Candidate> reasons{Candidate{places[0], digit}};
                for (const Candidate &value : ValuesShutting(grid, digit, std::move(others)))
                {
                    reasons.push_back(value);
                }
                return Step{Technique::kHiddenSingle,
                            "Hidden single: " + std::to_string(digit) + " has one place in " + unit.Name() + ", " +
                                geometry::Name(places[0]),
                            {Candidate{places[0], digit}},
                            {},
                            std::vector<int>(unit.cells.begin(), unit.cells.end()),
                            std::move(reasons)};
            }
        }
    }
    return std::nullopt;
}

} // namespace sudoku::techniques
