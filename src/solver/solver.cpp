#include "solver/solver.h"

#include "solver/techniques/techniques.h"

namespace sudoku
{

const std::vector<Solver::Finder> &Solver::Finders()
{
    static const std::vector<Finder> finders{
        techniques::NakedSingle, techniques::HiddenSingle, techniques::Pairs,        techniques::Intersections,
        techniques::Triples,     techniques::Quads,        techniques::XWing,        techniques::Swordfish,
        techniques::Jellyfish,   techniques::XYWing,       techniques::XYZWing,      techniques::SimpleColouring,
        techniques::XYChain,     techniques::Nishio,       techniques::ForcingChain, techniques::ContradictionChain};
    return finders;
}

std::optional<Step> Solver::NextStep()
{
    if (grid_.IsComplete() || Contradiction().has_value())
    {
        return std::nullopt;
    }
    for (const Finder &finder : Finders())
    {
        if (std::optional<Step> step = finder(grid_))
        {
            Apply(grid_, *step);
            return step;
        }
    }
    return std::nullopt;
}

std::vector<Step> Solver::Clue()
{
    std::vector<Step> steps;
    while (std::optional<Step> step = NextStep())
    {
        steps.push_back(std::move(*step));
        if (steps.back().IsPlacement())
        {
            break;
        }
    }
    return steps;
}

std::vector<Step> Solver::Solve()
{
    std::vector<Step> steps;
    while (std::optional<Step> step = NextStep())
    {
        steps.push_back(std::move(*step));
    }
    return steps;
}

void Solver::Apply(Grid &grid, const Step &step)
{
    for (const Candidate &placement : step.placements)
    {
        grid.Place(placement.cell, placement.digit);
    }
    for (const Candidate &elimination : step.eliminations)
    {
        grid.Eliminate(elimination.cell, elimination.digit);
    }
}

} // namespace sudoku
