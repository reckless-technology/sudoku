#include "solver/digits.h"
#include "solver/techniques/combinations.h"
#include "solver/techniques/marks.h"
#include "solver/techniques/techniques.h"

#include <algorithm>
#include <array>
#include <vector>

namespace sudoku::techniques
{

namespace
{

/** A base line with its places for the digit. */
struct BaseLine
{
    const Unit      *line{nullptr}; ///< The line.
    int              covers{0};     ///< The cover lines its places lie on, a bit per cover line.
    std::vector<int> places;        ///< Its places for the digit.
};

/**
 * @param cell A square.
 * @param is_row_based Whether the base lines are rows.
 * @return The cover line it lies on: its column for row bases, its row for column bases.
 */
int CoverIndex(int cell, bool is_row_based)
{
    return is_row_based ? geometry::Column(cell) : geometry::Row(cell);
}

/**
 * @param cell A square.
 * @param is_row_based Whether the base lines are rows.
 * @return The base line it lies on: its row for row bases, its column for column bases.
 */
int BaseIndex(int cell, bool is_row_based)
{
    return is_row_based ? geometry::Row(cell) : geometry::Column(cell);
}

/**
 * @param grid The board.
 * @param size How many lines: 2, 3 or 4.
 * @param technique The fish of that size.
 * @return The first fish of that size that strikes a candidate, the digits in order, each on rows then on columns.
 */
std::optional<Step> FindFish(const Grid &grid, int size, Technique technique)
{
    for (int digit = 1; digit <= 9; digit++)
    {
        for (bool is_row_based : {true, false})
        {
            const std::array<Unit, 9> &bases  = is_row_based ? geometry::Rows() : geometry::Columns();
            const std::array<Unit, 9> &covers = is_row_based ? geometry::Columns() : geometry::Rows();
            std::vector<BaseLine>      candidates;
            for (const Unit &line : bases)
            {
                if (grid.HasPlaced(line, digit))
                {
                    continue;
                }
                std::vector<int> places = grid.PlacesFor(line, digit);
                if (places.size() >= 2 && static_cast<int>(places.size()) <= size)
                {
                    int cover_set = 0;
                    for (int cell : places)
                    {
                        cover_set |= 1 << CoverIndex(cell, is_row_based);
                    }
                    candidates.push_back(BaseLine{&line, cover_set, std::move(places)});
                }
            }
            for (const std::vector<BaseLine> &fish : Combinations(candidates, size))
            {
                int cover_set = 0;
                for (const BaseLine &base : fish)
                {
                    cover_set |= base.covers;
                }
                if (digits::Count(cover_set) != size)
                {
                    continue;
                }
                auto is_base = [&](int cell) {
                    return std::any_of(fish.begin(), fish.end(), [&](const BaseLine &base) {
                        return base.line->index == BaseIndex(cell, is_row_based);
                    });
                };
                std::vector<Candidate> eliminations;
                for (int cover = 0; cover < 9; cover++)
                {
                    if ((cover_set & 1 << cover) == 0)
                    {
                        continue;
                    }
                    for (int cell : grid.PlacesFor(covers[static_cast<std::size_t>(cover)], digit))
                    {
                        if (!is_base(cell))
                        {
                            eliminations.push_back(Candidate{cell, digit});
                        }
                    }
                }
                if (eliminations.empty())
                {
                    continue;
                }
                std::vector<int> pattern;
                std::string      base_names;
                for (const BaseLine &base : fish)
                {
                    pattern.insert(pattern.end(), base.places.begin(), base.places.end());
                    base_names += (base_names.empty() ? "" : ", ") + base.line->Name();
                }
                std::sort(pattern.begin(), pattern.end());
                std::string cover_names;
                for (int cover = 0; cover < 9; cover++)
                {
                    if ((cover_set & 1 << cover) != 0)
                    {
                        cover_names +=
                            (cover_names.empty() ? "" : ", ") + covers[static_cast<std::size_t>(cover)].Name();
                    }
                }
                std::string description = std::string(TechniqueName(technique)) + ": " + std::to_string(digit) +
                                          " in " + base_names + " lies only in " + cover_names + " (" +
                                          geometry::Names(pattern) + "), so the rest of those lines " +
                                          Removes(eliminations);
                return Step{technique, std::move(description),
                            {},        std::move(eliminations),
                            pattern,   MarksIn(grid, pattern, digits::Bit(digit))};
            }
        }
    }
    return std::nullopt;
}

} // namespace

std::optional<Step> XWing(const Grid &grid)
{
    return FindFish(grid, 2, Technique::kXWing);
}

std::optional<Step> Swordfish(const Grid &grid)
{
    return FindFish(grid, 3, Technique::kSwordfish);
}

std::optional<Step> Jellyfish(const Grid &grid)
{
    return FindFish(grid, 4, Technique::kJellyfish);
}

} // namespace sudoku::techniques
