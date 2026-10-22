#include "solver/digits.h"
#include "solver/techniques/combinations.h"
#include "solver/techniques/marks.h"
#include "solver/techniques/techniques.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <string_view>
#include <vector>

namespace sudoku::techniques
{

namespace
{

/** The subsets' names by size. */
constexpr std::array<std::string_view, 5> kSizeNames{"", "single", "pair", "triple", "quad"};

/**
 * @param size 2, 3 or 4.
 * @return The naked subset of that size.
 */
Technique NakedTechnique(int size)
{
    return size == 2 ? Technique::kNakedPair : size == 3 ? Technique::kNakedTriple : Technique::kNakedQuad;
}

/**
 * @param size 2, 3 or 4.
 * @return The hidden subset of that size.
 */
Technique HiddenTechnique(int size)
{
    return size == 2 ? Technique::kHiddenPair : size == 3 ? Technique::kHiddenTriple : Technique::kHiddenQuad;
}

} // namespace

std::optional<Step> NakedSubset(const Grid &grid, int size)
{
    for (const Unit &unit : geometry::Units())
    {
        std::vector<int> empty;
        std::copy_if(unit.cells.begin(), unit.cells.end(), std::back_inserter(empty),
                     [&](int cell) { return grid.IsEmpty(cell); });
        if (static_cast<int>(empty.size()) <= size)
        {
            continue; // the unit's every empty square: nothing outside it to strike from
        }
        std::vector<int> small;
        std::copy_if(empty.begin(), empty.end(), std::back_inserter(small),
                     [&](int cell) { return digits::Count(grid.Candidates(cell)) <= size; });
        for (const std::vector<int> &subset : Combinations(small, size))
        {
            int set = 0;
            for (int cell : subset)
            {
                set |= grid.Candidates(cell);
            }
            if (digits::Count(set) != size)
            {
                continue;
            }
            std::vector<Candidate> eliminations;
            for (int cell : empty)
            {
                if (std::find(subset.begin(), subset.end(), cell) != subset.end())
                {
                    continue;
                }
                for (int digit : digits::Each(grid.Candidates(cell) & set))
                {
                    eliminations.push_back(Candidate{cell, digit});
                }
            }
            if (!eliminations.empty())
            {
                std::string description = "Naked " + std::string(kSizeNames[static_cast<std::size_t>(size)]) + " " +
                                          digits::Format(set) + " in " + unit.Name() + " (" + geometry::Names(subset) +
                                          ") " + Removes(eliminations);
                return Step{NakedTechnique(size),      std::move(description), {}, std::move(eliminations), subset,
                            MarksIn(grid, subset, set)};
            }
        }
    }
    return std::nullopt;
}

std::optional<Step> HiddenSubset(const Grid &grid, int size)
{
    for (const Unit &unit : geometry::Units())
    {
        std::vector<int> open;
        for (int digit = 1; digit <= 9; digit++)
        {
            if (!grid.HasPlaced(unit, digit))
            {
                open.push_back(digit);
            }
        }
        if (static_cast<int>(open.size()) <= size)
        {
            continue; // every digit the unit lacks: nothing else to strike
        }
        std::array<std::vector<int>, 10> places{};
        std::vector<int>                 few;
        for (int digit : open)
        {
            places[static_cast<std::size_t>(digit)] = grid.PlacesFor(unit, digit);
            int count                               = static_cast<int>(places[static_cast<std::size_t>(digit)].size());
            if (count >= 1 && count <= size)
            {
                few.push_back(digit);
            }
        }
        for (const std::vector<int> &subset : Combinations(few, size))
        {
            std::vector<int> cells;
            int              set = 0;
            for (int digit : subset)
            {
                const std::vector<int> &digit_places = places[static_cast<std::size_t>(digit)];
                cells.insert(cells.end(), digit_places.begin(), digit_places.end());
                set |= digits::Bit(digit);
            }
            std::sort(cells.begin(), cells.end());
            cells.erase(std::unique(cells.begin(), cells.end()), cells.end());
            if (static_cast<int>(cells.size()) != size)
            {
                continue;
            }
            std::vector<Candidate> eliminations;
            for (int cell : cells)
            {
                for (int digit : digits::Each(grid.Candidates(cell) & ~set))
                {
                    eliminations.push_back(Candidate{cell, digit});
                }
            }
            if (!eliminations.empty())
            {
                std::string description = "Hidden " + std::string(kSizeNames[static_cast<std::size_t>(size)]) + " " +
                                          digits::Format(set) + " in " + unit.Name() + " (" + geometry::Names(cells) +
                                          ") " + Removes(eliminations);
                return Step{HiddenTechnique(size),    std::move(description), {}, std::move(eliminations), cells,
                            MarksIn(grid, cells, set)};
            }
        }
    }
    return std::nullopt;
}

std::optional<Step> Pairs(const Grid &grid)
{
    if (std::optional<Step> step = NakedSubset(grid, 2))
    {
        return step;
    }
    return HiddenSubset(grid, 2);
}

std::optional<Step> Triples(const Grid &grid)
{
    if (std::optional<Step> step = NakedSubset(grid, 3))
    {
        return step;
    }
    return HiddenSubset(grid, 3);
}

std::optional<Step> Quads(const Grid &grid)
{
    if (std::optional<Step> step = NakedSubset(grid, 4))
    {
        return step;
    }
    return HiddenSubset(grid, 4);
}

} // namespace sudoku::techniques
