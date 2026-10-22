#include "solver/digits.h"
#include "solver/techniques/techniques.h"

#include <algorithm>
#include <vector>

namespace sudoku::techniques
{

namespace
{

/**
 * One digit's hidden singles placed until none is left, or a unit has no place for it.
 * @param grid The board, changed.
 * @param digit The digit.
 * @return The unit left with no place, in words; none for none.
 */
std::optional<std::string> FollowDigit(Grid &grid, int digit)
{
    bool has_placed = true;
    while (has_placed)
    {
        has_placed = false;
        for (const Unit &unit : geometry::Units())
        {
            if (grid.HasPlaced(unit, digit))
            {
                continue;
            }
            std::vector<int> places = grid.PlacesFor(unit, digit);
            if (places.empty())
            {
                return "no place for " + std::to_string(digit) + " in " + unit.Name();
            }
            if (places.size() == 1)
            {
                grid.Place(places[0], digit);
                has_placed = true;
            }
        }
    }
    return std::nullopt;
}

/**
 * @param grid The board.
 * @return The empty squares, those with the fewest candidates first, then in order.
 */
std::vector<int> EmptyByCandidateCount(const Grid &grid)
{
    std::vector<int> cells;
    for (int cell = 0; cell < geometry::kCellCount; cell++)
    {
        if (grid.IsEmpty(cell))
        {
            cells.push_back(cell);
        }
    }
    std::stable_sort(cells.begin(), cells.end(), [&](int a, int b) {
        return digits::Count(grid.Candidates(a)) < digits::Count(grid.Candidates(b));
    });
    return cells;
}

} // namespace

std::optional<Step> Nishio(const Grid &grid)
{
    for (int digit = 1; digit <= 9; digit++)
    {
        for (int cell = 0; cell < geometry::kCellCount; cell++)
        {
            if (!grid.IsEmpty(cell) || !digits::Contains(grid.Candidates(cell), digit))
            {
                continue;
            }
            Grid trial = grid;
            trial.Place(cell, digit);
            if (std::optional<std::string> reason = FollowDigit(trial, digit))
            {
                std::string name        = geometry::Name(cell);
                std::string description = "Nishio: were " + name + " " + std::to_string(digit) + ", " +
                                          std::to_string(digit) + "'s own singles leave " + *reason + ", so " + name +
                                          " is not " + std::to_string(digit);
                return Step{Technique::kNishio, std::move(description), {}, {Candidate{cell, digit}}, {cell}};
            }
        }
    }
    return std::nullopt;
}

std::optional<Step> ForcingChain(const Grid &grid)
{
    for (int cell : EmptyByCandidateCount(grid))
    {
        if (digits::Count(grid.Candidates(cell)) > 3)
        {
            continue;
        }
        std::vector<int>  choices = digits::Each(grid.Candidates(cell));
        std::vector<Grid> outcomes;
        for (int digit : choices)
        {
            Grid trial = grid;
            trial.Place(cell, digit);
            if (!Propagate(trial))
            {
                outcomes.push_back(trial);
            }
        }
        if (outcomes.size() != choices.size())
        {
            continue; // a trial that fails is the contradiction chain's to report
        }
        std::string name        = geometry::Name(cell);
        std::string choice_text = digits::Format(grid.Candidates(cell));
        for (int other = 0; other < geometry::kCellCount; other++)
        {
            int value = outcomes[0].Value(other);
            if (grid.IsEmpty(other) && value != 0 &&
                std::all_of(outcomes.begin(), outcomes.end(),
                            [&](const Grid &outcome) { return outcome.Value(other) == value; }))
            {
                std::string description = "Forcing chain: whichever of " + choice_text + " " + name +
                                          " takes, the singles that follow give " + geometry::Name(other) + " = " +
                                          std::to_string(value);
                return Step{
                    Technique::kForcingChain, std::move(description), {Candidate{other, value}}, {}, {cell, other}};
            }
        }
        std::vector<Candidate> eliminations;
        for (int other = 0; other < geometry::kCellCount; other++)
        {
            if (!grid.IsEmpty(other))
            {
                continue;
            }
            for (int digit : digits::Each(grid.Candidates(other)))
            {
                if (std::none_of(outcomes.begin(), outcomes.end(), [&](const Grid &outcome) {
                        return digits::Contains(outcome.Candidates(other), digit);
                    }))
                {
                    eliminations.push_back(Candidate{other, digit});
                }
            }
        }
        if (!eliminations.empty())
        {
            // "removes 3 from a9" as "remove 3 from a9": the singles remove.
            std::string removes     = Removes(eliminations);
            std::string description = "Forcing chain: whichever of " + choice_text + " " + name +
                                      " takes, the singles that follow remove" + removes.substr(7);
            return Step{Technique::kForcingChain, std::move(description), {}, std::move(eliminations), {cell}};
        }
    }
    return std::nullopt;
}

std::optional<Step> ContradictionChain(const Grid &grid)
{
    for (int cell : EmptyByCandidateCount(grid))
    {
        for (int digit : digits::Each(grid.Candidates(cell)))
        {
            Grid trial = grid;
            trial.Place(cell, digit);
            if (std::optional<std::string> reason = Propagate(trial))
            {
                std::string name        = geometry::Name(cell);
                std::string description = "Contradiction chain: were " + name + " " + std::to_string(digit) +
                                          ", the singles that follow leave " + *reason + ", so " + name + " is not " +
                                          std::to_string(digit);
                return Step{
                    Technique::kContradictionChain, std::move(description), {}, {Candidate{cell, digit}}, {cell}};
            }
        }
    }
    return std::nullopt;
}

std::optional<std::string> Propagate(Grid &grid)
{
    while (true)
    {
        if (std::optional<std::string> reason = grid.FindContradiction())
        {
            return reason;
        }
        std::optional<Step> single = NakedSingle(grid);
        if (!single)
        {
            single = HiddenSingle(grid);
        }
        if (!single)
        {
            return std::nullopt;
        }
        for (const Candidate &placement : single->placements)
        {
            grid.Place(placement.cell, placement.digit);
        }
    }
}

} // namespace sudoku::techniques
