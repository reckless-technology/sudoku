/// @file puzzle_test.cpp Every puzzle in puzzles/: one solution by search, and the solver reaching it by deduction -
/// every value it places the solution's, every candidate it strikes not the solution's - with every technique used
/// somewhere.

#include "puzzles.h"

#include "solver/brute_force.h"
#include "solver/puzzle_file.h"
#include "solver/solver.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <set>
#include <string>
#include <vector>

namespace sudoku
{
namespace
{

/** The techniques no puzzle here calls for, each held to its own suite instead (technique_test.cpp). */
const std::vector<Technique> kUnusedByThePuzzles{Technique::kNakedQuad, Technique::kJellyfish, Technique::kForcingChain,
                                                 Technique::kContradictionChain};

TEST(Puzzles, ThePuzzlesAreThere)
{
    EXPECT_EQ(test::PuzzleNames().size(), 68u);
}

/** A puzzle, by file name. */
class EachPuzzle : public testing::TestWithParam<std::string>
{
};

TEST_P(EachPuzzle, IsSolvedByDeductionAndEveryStepIsTrue)
{
    const std::string &name   = GetParam();
    Values             givens = puzzle_file::Read(test::PuzzlesFolder() / name);
    Values             solution{};
    ASSERT_EQ(CountSolutions(givens, &solution), SolutionCount::kOne);

    Grid   grid = Grid::FromValues(givens);
    Solver solver(grid);
    while (std::optional<Step> step = solver.NextStep())
    {
        for (const Candidate &placement : step->placements)
        {
            EXPECT_EQ(solution[static_cast<std::size_t>(placement.cell)], placement.digit)
                << name << ": " << step->description << " places " << placement.Text();
        }
        for (const Candidate &elimination : step->eliminations)
        {
            EXPECT_NE(solution[static_cast<std::size_t>(elimination.cell)], elimination.digit)
                << name << ": " << step->description << " strikes the solution's " << elimination.Text();
        }
        EXPECT_NE(step->description.find_first_not_of(" \t\r\n"), std::string::npos);
        EXPECT_FALSE(step->pattern.empty());
    }
    EXPECT_TRUE(solver.grid().IsComplete())
        << name << ": stuck with " << std::count(grid.values().begin(), grid.values().end(), 0) << " squares empty";
    EXPECT_EQ(solver.grid().values(), solution);
}

INSTANTIATE_TEST_SUITE_P(Puzzles, EachPuzzle, testing::ValuesIn(test::PuzzleNames()),
                         [](const testing::TestParamInfo<std::string> &info) { return test::TestName(info.param); });

TEST(Puzzles, EveryTechniqueIsUsedOnSomePuzzle)
{
    std::set<Technique> used;
    for (const std::string &name : test::PuzzleNames())
    {
        Grid grid = Grid::FromValues(puzzle_file::Read(test::PuzzlesFolder() / name));
        for (const Step &step : Solver(grid).Solve())
        {
            used.insert(step.technique);
        }
    }
    std::string missing;
    for (Technique technique : kTechniques)
    {
        bool is_unused =
            std::find(kUnusedByThePuzzles.begin(), kUnusedByThePuzzles.end(), technique) != kUnusedByThePuzzles.end();
        if (!is_unused && !used.contains(technique))
        {
            missing += (missing.empty() ? "" : ", ") + std::string(TechniqueName(technique));
        }
        EXPECT_FALSE(is_unused && used.contains(technique)) << "a puzzle uses " << TechniqueName(technique);
    }
    EXPECT_TRUE(missing.empty()) << "no puzzle uses " << missing;
}

} // namespace
} // namespace sudoku
