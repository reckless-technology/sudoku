#include "solver/digits.h"
#include "solver/techniques/combinations.h"
#include "solver/techniques/marks.h"
#include "solver/techniques/seeing.h"
#include "solver/techniques/techniques.h"

#include <algorithm>
#include <array>
#include <vector>

namespace sudoku::techniques
{

namespace
{

/**
 * @param grid The board.
 * @param cell A square.
 * @return The empty squares with two candidates that share a unit with it, in order.
 */
std::vector<int> BivaluePeers(const Grid &grid, int cell)
{
    std::vector<int> peers;
    for (int peer : geometry::Peers(cell))
    {
        if (grid.IsEmpty(peer) && digits::Count(grid.Candidates(peer)) == 2)
        {
            peers.push_back(peer);
        }
    }
    return peers;
}

} // namespace

std::vector<Candidate> Seeing(const Grid &grid, int digit, std::span<const int> seen, std::span<const int> also)
{
    std::vector<Candidate> eliminations;
    for (int cell = 0; cell < geometry::kCellCount; cell++)
    {
        if (grid.IsEmpty(cell) && digits::Contains(grid.Candidates(cell), digit) &&
            std::find(seen.begin(), seen.end(), cell) == seen.end() &&
            std::find(also.begin(), also.end(), cell) == also.end() &&
            std::all_of(seen.begin(), seen.end(), [&](int other) { return geometry::Sees(cell, other); }))
        {
            eliminations.push_back(Candidate{cell, digit});
        }
    }
    return eliminations;
}

std::optional<Step> XYWing(const Grid &grid)
{
    for (int pivot = 0; pivot < geometry::kCellCount; pivot++)
    {
        int pivot_set = grid.Candidates(pivot);
        if (!grid.IsEmpty(pivot) || digits::Count(pivot_set) != 2)
        {
            continue;
        }
        std::vector<int> pincers = BivaluePeers(grid, pivot);
        for (int first : pincers)
        {
            int first_set = grid.Candidates(first);
            int shared    = first_set & pivot_set;
            int z         = first_set & ~pivot_set;
            if (digits::Count(shared) != 1 || digits::Count(z) != 1)
            {
                continue;
            }
            int wanted = (pivot_set & ~shared) | z;
            for (int second : pincers)
            {
                if (second == first || grid.Candidates(second) != wanted)
                {
                    continue;
                }
                int                      digit = digits::First(z);
                const std::array<int, 2> ends{first, second};
                const std::array<int, 1> centre{pivot};
                std::vector<Candidate>   eliminations = Seeing(grid, digit, ends, centre);
                if (!eliminations.empty())
                {
                    std::string description = "XY-wing: pivot " + geometry::Name(pivot) + " " +
                                              digits::Format(pivot_set) + " with pincers " + geometry::Name(first) +
                                              " " + digits::Format(first_set) + " and " + geometry::Name(second) + " " +
                                              digits::Format(wanted) + ": one pincer is " + std::to_string(digit) +
                                              ", so what sees both pincers loses it - " + Removes(eliminations);
                    return Step{Technique::kXYWing,
                                std::move(description),
                                {},
                                std::move(eliminations),
                                {pivot, first, second},
                                MarksIn(grid, std::array<int, 3>{pivot, first, second}, digits::kAll)};
                }
            }
        }
    }
    return std::nullopt;
}

std::optional<Step> XYZWing(const Grid &grid)
{
    for (int pivot = 0; pivot < geometry::kCellCount; pivot++)
    {
        int pivot_set = grid.Candidates(pivot);
        if (!grid.IsEmpty(pivot) || digits::Count(pivot_set) != 3)
        {
            continue;
        }
        std::vector<int> pincers;
        for (int peer : BivaluePeers(grid, pivot))
        {
            if ((grid.Candidates(peer) & ~pivot_set) == 0)
            {
                pincers.push_back(peer);
            }
        }
        for (const std::vector<int> &pair : Combinations(pincers, 2))
        {
            int first_set  = grid.Candidates(pair[0]);
            int second_set = grid.Candidates(pair[1]);
            int z          = first_set & second_set;
            if (first_set == second_set || digits::Count(z) != 1 || (first_set | second_set) != pivot_set)
            {
                continue;
            }
            int                      digit = digits::First(z);
            const std::array<int, 3> three{pivot, pair[0], pair[1]};
            std::vector<Candidate>   eliminations = Seeing(grid, digit, three, {});
            if (!eliminations.empty())
            {
                std::string description = "XYZ-wing: pivot " + geometry::Name(pivot) + " " + digits::Format(pivot_set) +
                                          " with pincers " + geometry::Name(pair[0]) + " " + digits::Format(first_set) +
                                          " and " + geometry::Name(pair[1]) + " " + digits::Format(second_set) +
                                          ": one of the three is " + std::to_string(digit) +
                                          ", so what sees all three loses it - " + Removes(eliminations);
                return Step{Technique::kXYZWing,     std::move(description),    {},
                            std::move(eliminations), {pivot, pair[0], pair[1]}, MarksIn(grid, three, digits::kAll)};
            }
        }
    }
    return std::nullopt;
}

} // namespace sudoku::techniques
