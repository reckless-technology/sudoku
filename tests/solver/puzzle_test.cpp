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
    const std::string &name = GetParam();
    test::ExpectSolvedByDeduction(puzzle_file::Read(test::PuzzlesFolder() / name), name);
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
