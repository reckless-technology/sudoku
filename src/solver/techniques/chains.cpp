#include "solver/digits.h"
#include "solver/techniques/combinations.h"
#include "solver/techniques/seeing.h"
#include "solver/techniques/techniques.h"

#include <algorithm>
#include <array>
#include <deque>
#include <map>
#include <vector>

namespace sudoku::techniques
{

namespace
{

/**
 * Links one square to another, once.
 * @param links Each square's linked squares, in the order they were linked.
 * @param from A square.
 * @param to The square it is linked to.
 */
void Link(std::map<int, std::vector<int>> &links, int from, int to)
{
    std::vector<int> &list = links[from];
    if (std::find(list.begin(), list.end(), to) == list.end())
    {
        list.push_back(to);
    }
}

/**
 * @param cells Squares.
 * @return Whether two of them see each other.
 */
bool HasTwoInAUnit(const std::vector<int> &cells)
{
    for (const std::vector<int> &pair : Combinations(cells, 2))
    {
        if (geometry::Sees(pair[0], pair[1]))
        {
            return true;
        }
    }
    return false;
}

/**
 * @param cell A square.
 * @param cells Squares.
 * @return Whether it sees any of them.
 */
bool SeesAny(int cell, const std::vector<int> &cells)
{
    return std::any_of(cells.begin(), cells.end(), [&](int other) { return geometry::Sees(cell, other); });
}

/**
 * @param squares A value for each square.
 * @param cell A square.
 * @return Its value.
 */
int &At(std::array<int, geometry::kCellCount> &squares, int cell)
{
    return squares[static_cast<std::size_t>(cell)];
}

} // namespace

std::optional<Step> SimpleColouring(const Grid &grid)
{
    for (int digit = 1; digit <= 9; digit++)
    {
        std::map<int, std::vector<int>> links;
        for (const Unit &unit : geometry::Units())
        {
            std::vector<int> places = grid.PlacesFor(unit, digit);
            if (places.size() == 2 && !grid.HasPlaced(unit, digit))
            {
                Link(links, places[0], places[1]);
                Link(links, places[1], places[0]);
            }
        }
        std::array<int, geometry::kCellCount> colours{};
        colours.fill(-1);
        for (const auto &entry : links)
        {
            int start = entry.first;
            if (At(colours, start) >= 0)
            {
                continue;
            }
            std::vector<int> component{start};
            At(colours, start) = 0;
            std::deque<int> queue{start};
            while (!queue.empty())
            {
                int cell = queue.front();
                queue.pop_front();
                for (int next : links.at(cell))
                {
                    if (At(colours, next) >= 0)
                    {
                        continue;
                    }
                    At(colours, next) = 1 - At(colours, cell);
                    component.push_back(next);
                    queue.push_back(next);
                }
            }
            std::sort(component.begin(), component.end());
            std::array<std::vector<int>, 2> by_colour;
            for (int cell : component)
            {
                by_colour[static_cast<std::size_t>(At(colours, cell))].push_back(cell);
            }
            std::string chain = geometry::Names(by_colour[0]) + " | " + geometry::Names(by_colour[1]);
            for (const std::vector<int> &same : by_colour)
            {
                if (!HasTwoInAUnit(same))
                {
                    continue;
                }
                std::vector<Candidate> eliminations;
                for (int cell : same)
                {
                    eliminations.push_back(Candidate{cell, digit});
                }
                std::string description = "Simple colouring (wrap): " + std::to_string(digit) + "'s chain " + chain +
                                          " has two squares of one colour in a unit, so that colour is false - " +
                                          Removes(eliminations);
                return Step{
                    Technique::kSimpleColouring, std::move(description), {}, std::move(eliminations), component};
            }
            std::vector<Candidate> trapped;
            for (int cell = 0; cell < geometry::kCellCount; cell++)
            {
                if (grid.IsEmpty(cell) && digits::Contains(grid.Candidates(cell), digit) && At(colours, cell) < 0 &&
                    SeesAny(cell, by_colour[0]) && SeesAny(cell, by_colour[1]))
                {
                    trapped.push_back(Candidate{cell, digit});
                }
            }
            if (!trapped.empty())
            {
                std::string description = "Simple colouring (trap): " + std::to_string(digit) + "'s chain " + chain +
                                          " - one colour holds it, so what sees both colours loses it - " +
                                          Removes(trapped);
                return Step{Technique::kSimpleColouring, std::move(description), {}, std::move(trapped), component};
            }
        }
    }
    return std::nullopt;
}

std::optional<Step> XYChain(const Grid &grid)
{
    std::vector<int> bivalue;
    for (int cell = 0; cell < geometry::kCellCount; cell++)
    {
        if (grid.IsEmpty(cell) && digits::Count(grid.Candidates(cell)) == 2)
        {
            bivalue.push_back(cell);
        }
    }
    for (int start : bivalue)
    {
        for (int z : digits::Each(grid.Candidates(start)))
        {
            // Breadth first, so the shortest chain is found: each square's digit if the start is not z, 0 for a square
            // not reached.
            std::array<int, geometry::kCellCount> holds{};
            std::array<int, geometry::kCellCount> previous{};
            At(holds, start) = digits::First(grid.Candidates(start) & ~digits::Bit(z));
            std::deque<int> queue{start};
            while (!queue.empty())
            {
                int cell = queue.front();
                queue.pop_front();
                int digit = At(holds, cell);
                for (int next : bivalue)
                {
                    if (At(holds, next) != 0 || !geometry::Sees(cell, next) ||
                        !digits::Contains(grid.Candidates(next), digit))
                    {
                        continue;
                    }
                    At(holds, next)    = digits::First(grid.Candidates(next) & ~digits::Bit(digit));
                    At(previous, next) = cell;
                    queue.push_back(next);
                    if (At(holds, next) != z)
                    {
                        continue;
                    }
                    std::vector<int> chain{next};
                    while (chain.back() != start)
                    {
                        chain.push_back(At(previous, chain.back()));
                    }
                    std::reverse(chain.begin(), chain.end());
                    if (chain.size() < 3)
                    {
                        continue; // two squares that see each other: a naked pair
                    }
                    const std::array<int, 2> ends{start, next};
                    std::vector<Candidate>   eliminations = Seeing(grid, z, ends, chain);
                    if (eliminations.empty())
                    {
                        continue;
                    }
                    std::string links;
                    for (int link : chain)
                    {
                        links +=
                            (links.empty() ? "" : " - ") + geometry::Name(link) + digits::Format(grid.Candidates(link));
                    }
                    std::string description = "XY-chain " + links + ": one end is " + std::to_string(z) +
                                              ", so what sees both ends loses it - " + Removes(eliminations);
                    return Step{
                        Technique::kXYChain, std::move(description), {}, std::move(eliminations), std::move(chain)};
                }
            }
        }
    }
    return std::nullopt;
}

} // namespace sudoku::techniques
