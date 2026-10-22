/// @file technique_test.cpp Each technique on its own: called directly on every puzzle as it is solved, whatever it
/// finds must be true - so a technique the solver's order rarely reaches is held to the solution all the same - with
/// every reason it gives a candidate or a value of the board it read; and found where a pattern is built for it, with
/// the marks and values it read.

#include "puzzles.h"

#include "solver/brute_force.h"
#include "solver/digits.h"
#include "solver/puzzle_file.h"
#include "solver/solver.h"
#include "solver/techniques/combinations.h"
#include "solver/techniques/techniques.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <initializer_list>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace sudoku
{
namespace
{

/** A technique, by name, with how often to try it: every n-th board on the way, the slow ones less often. */
struct NamedFinder
{
    std::string    name;   ///< The technique's name, as a test's name.
    Solver::Finder finder; ///< The technique.
    int            every;  ///< Tried on every n-th board.
};

/**
 * Prints a technique as its name, for the test's report.
 * @param finder The technique.
 * @param stream Where to print it.
 */
void PrintTo(const NamedFinder &finder, std::ostream *stream)
{
    *stream << finder.name;
}

/** @return Every technique, by name; the forcing ones tried on every fourth or eighth board, they being slow. */
std::vector<NamedFinder> Finders()
{
    return {
        {"NakedSingle", techniques::NakedSingle, 1},
        {"HiddenSingle", techniques::HiddenSingle, 1},
        {"NakedPair", [](const Grid &grid) { return techniques::NakedSubset(grid, 2); }, 1},
        {"HiddenPair", [](const Grid &grid) { return techniques::HiddenSubset(grid, 2); }, 1},
        {"Pointing", techniques::Pointing, 1},
        {"Claiming", techniques::Claiming, 1},
        {"NakedTriple", [](const Grid &grid) { return techniques::NakedSubset(grid, 3); }, 1},
        {"HiddenTriple", [](const Grid &grid) { return techniques::HiddenSubset(grid, 3); }, 1},
        {"NakedQuad", [](const Grid &grid) { return techniques::NakedSubset(grid, 4); }, 1},
        {"HiddenQuad", [](const Grid &grid) { return techniques::HiddenSubset(grid, 4); }, 1},
        {"XWing", techniques::XWing, 1},
        {"Swordfish", techniques::Swordfish, 1},
        {"Jellyfish", techniques::Jellyfish, 1},
        {"XYWing", techniques::XYWing, 1},
        {"XYZWing", techniques::XYZWing, 1},
        {"SimpleColouring", techniques::SimpleColouring, 1},
        {"XYChain", techniques::XYChain, 1},
        {"Nishio", techniques::Nishio, 4},
        {"ForcingChain", techniques::ForcingChain, 8},
        {"ContradictionChain", techniques::ContradictionChain, 8},
    };
}

/** A technique, by name. */
class EachTechnique : public testing::TestWithParam<NamedFinder>
{
};

TEST_P(EachTechnique, WhatItFindsAnywhereIsTrue)
{
    const NamedFinder &finder = GetParam();
    int                found  = 0;
    // The repository's puzzles, and the first diabolical ones of the Sudoku Exchange sample, whose boards the chains
    // and the forcing techniques meet far more often.
    std::vector<std::pair<std::string, Values>> puzzles;
    for (const std::string &name : test::PuzzleNames())
    {
        puzzles.emplace_back(name, puzzle_file::Read(test::PuzzlesFolder() / name));
    }
    std::vector<test::RatedPuzzle> diabolical = test::RatedPuzzles("diabolical");
    diabolical.resize(std::min<std::size_t>(diabolical.size(), 30));
    for (const test::RatedPuzzle &puzzle : diabolical)
    {
        puzzles.emplace_back("diabolical " + puzzle.id, puzzle.Givens());
    }
    for (const auto &[name, givens] : puzzles)
    {
        Values solution{};
        ASSERT_EQ(CountSolutions(givens, &solution), SolutionCount::kOne) << name;
        Grid   grid = Grid::FromValues(givens);
        Solver solver(grid);
        int    board = 0;
        do
        {
            if (board++ % finder.every != 0)
            {
                continue;
            }
            Grid                before = grid;
            std::optional<Step> step   = finder.finder(before);
            if (!step)
            {
                continue;
            }
            found++;
            EXPECT_EQ(before, grid) << name << ": finding changes nothing";
            for (const Candidate &placement : step->placements)
            {
                EXPECT_EQ(solution[static_cast<std::size_t>(placement.cell)], placement.digit)
                    << name << ": " << step->description;
                EXPECT_TRUE(grid.IsEmpty(placement.cell))
                    << name << ": " << step->description << " fills a full square";
            }
            for (const Candidate &elimination : step->eliminations)
            {
                EXPECT_NE(solution[static_cast<std::size_t>(elimination.cell)], elimination.digit)
                    << name << ": " << step->description;
                EXPECT_TRUE(digits::Contains(grid.Candidates(elimination.cell), elimination.digit))
                    << name << ": " << step->description << " strikes what is already gone";
            }
            EXPECT_TRUE(step->IsPlacement() || !step->eliminations.empty()) << step->description << " does nothing";
            EXPECT_FALSE(step->reasons.empty()) << name << ": " << step->description << " reads nothing";
            for (const Candidate &reason : step->reasons)
            {
                const bool is_mark =
                    grid.IsEmpty(reason.cell) && digits::Contains(grid.Candidates(reason.cell), reason.digit);
                const bool is_value = grid.Value(reason.cell) == reason.digit;
                EXPECT_TRUE(is_mark || is_value) << name << ": " << step->description << " reads " << reason.Text()
                                                 << ", which the board does not hold";
            }
        } while (solver.NextStep());
    }
    EXPECT_GT(found, 0) << finder.name << " found nothing on any puzzle";
}

INSTANTIATE_TEST_SUITE_P(Techniques, EachTechnique, testing::ValuesIn(Finders()),
                         [](const testing::TestParamInfo<NamedFinder> &info) { return info.param.name; });

/**
 * A square's candidates cut to the digits given.
 * @param grid The board.
 * @param cell The square.
 * @param kept The digits it keeps.
 */
void Restrict(Grid &grid, int cell, std::initializer_list<int> kept)
{
    for (int digit = 1; digit <= 9; digit++)
    {
        if (std::find(kept.begin(), kept.end(), digit) == kept.end())
        {
            grid.Eliminate(cell, digit);
        }
    }
}

/**
 * @param eliminations Candidates.
 * @return Them in order, by square then by digit.
 */
std::vector<Candidate> Sorted(std::vector<Candidate> eliminations)
{
    std::sort(eliminations.begin(), eliminations.end());
    return eliminations;
}

TEST(Techniques, ANakedQuadNeedNotHaveEveryDigitInEverySquare)
{
    Grid grid;
    // Row 9's first four squares, a9 to d9: {1,2}, {2,3}, {3,4}, {1,4} - no pair or triple among them, a quad together.
    Restrict(grid, 0, {1, 2});
    Restrict(grid, 1, {2, 3});
    Restrict(grid, 2, {3, 4});
    Restrict(grid, 3, {1, 4});
    EXPECT_FALSE(techniques::NakedSubset(grid, 2));
    EXPECT_FALSE(techniques::NakedSubset(grid, 3));
    std::optional<Step> step = techniques::Quads(grid);
    ASSERT_TRUE(step);
    EXPECT_EQ(step->technique, Technique::kNakedQuad);
    EXPECT_EQ(step->pattern, (std::vector<int>{0, 1, 2, 3}));
    EXPECT_EQ(step->reasons, (std::vector<Candidate>{{0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 3}, {2, 4}, {3, 1}, {3, 4}}));
    std::vector<Candidate> expected;
    for (int cell = 4; cell < 9; cell++)
    {
        for (int digit = 1; digit <= 4; digit++)
        {
            expected.push_back(Candidate{cell, digit});
        }
    }
    EXPECT_EQ(Sorted(step->eliminations), expected);
}

TEST(Techniques, ANakedTripleIsFoundWithNoSquareHoldingAllThree)
{
    Grid grid;
    Restrict(grid, 0, {1, 2});
    Restrict(grid, 1, {2, 3});
    Restrict(grid, 2, {1, 3});
    std::optional<Step> step = techniques::NakedSubset(grid, 3);
    ASSERT_TRUE(step);
    EXPECT_EQ(step->technique, Technique::kNakedTriple);
    EXPECT_NE(std::find(step->eliminations.begin(), step->eliminations.end(), Candidate{8, 1}),
              step->eliminations.end());
}

TEST(Techniques, AJellyfishStrikesTheDigitFromItsColumnsOutsideItsRows)
{
    Grid grid;
    // 1 only in columns a to d on rows 9 to 6.
    for (int row = 0; row < 4; row++)
    {
        for (int column = 4; column < 9; column++)
        {
            grid.Eliminate(geometry::Cell(row, column), 1);
        }
    }
    EXPECT_FALSE(techniques::XWing(grid));
    EXPECT_FALSE(techniques::Swordfish(grid));
    std::optional<Step> step = techniques::Jellyfish(grid);
    ASSERT_TRUE(step);
    EXPECT_EQ(step->technique, Technique::kJellyfish);
    std::vector<Candidate> expected;
    for (int row = 4; row < 9; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            expected.push_back(Candidate{geometry::Cell(row, column), 1});
        }
    }
    EXPECT_EQ(Sorted(step->eliminations), expected);
}

TEST(Techniques, AnXWingStrikesTheDigitFromItsColumnsOutsideItsRows)
{
    Grid grid;
    // 5 only in columns b and g on rows 7 and 2.
    for (int row : {2, 7})
    {
        for (int column = 0; column < 9; column++)
        {
            if (column != 1 && column != 6)
            {
                grid.Eliminate(geometry::Cell(row, column), 5);
            }
        }
    }
    std::optional<Step> step = techniques::XWing(grid);
    ASSERT_TRUE(step);
    EXPECT_EQ(step->technique, Technique::kXWing);
    EXPECT_EQ(step->reasons, (std::vector<Candidate>{{geometry::Cell(2, 1), 5},
                                                     {geometry::Cell(2, 6), 5},
                                                     {geometry::Cell(7, 1), 5},
                                                     {geometry::Cell(7, 6), 5}}))
        << "its four corners";
    EXPECT_EQ(step->eliminations.size(), 14u);
    for (const Candidate &elimination : step->eliminations)
    {
        int row    = geometry::Row(elimination.cell);
        int column = geometry::Column(elimination.cell);
        EXPECT_TRUE((column == 1 || column == 6) && row != 2 && row != 7) << elimination.Text();
    }
}

TEST(Techniques, AHiddenSingleReadsItsPlaceAndTheValuesThatShutTheOthers)
{
    Grid grid;
    // 1 in e8, h7, b5 and c2: row 9 has no place for 1 but a9.
    for (const int cell : {geometry::Cell(1, 4), geometry::Cell(2, 7), geometry::Cell(4, 1), geometry::Cell(7, 2)})
    {
        grid.Place(cell, 1);
    }
    std::optional<Step> step = techniques::HiddenSingle(grid);
    ASSERT_TRUE(step);
    EXPECT_EQ(step->technique, Technique::kHiddenSingle);
    EXPECT_EQ(step->placements, (std::vector<Candidate>{{0, 1}}));
    // The place, then the values that shut the most squares first: e8 (d9 to f9) and h7 (g9 to i9), then b5 and c2.
    EXPECT_EQ(step->reasons, (std::vector<Candidate>{{0, 1},
                                                     {geometry::Cell(1, 4), 1},
                                                     {geometry::Cell(2, 7), 1},
                                                     {geometry::Cell(4, 1), 1},
                                                     {geometry::Cell(7, 2), 1}}));
}

TEST(Techniques, AHiddenSinglesValuesAreTheFewestThatShutTheUnit)
{
    Grid grid;
    // 1 in e8, h7, a3, c2 and i5: row 9 has no place for it but b9. i5 shuts i9, which h7 shuts already.
    for (const int cell :
         {geometry::Cell(1, 4), geometry::Cell(2, 7), geometry::Cell(6, 0), geometry::Cell(7, 2), geometry::Cell(4, 8)})
    {
        grid.Place(cell, 1);
    }
    std::optional<Step> step = techniques::HiddenSingle(grid);
    ASSERT_TRUE(step);
    EXPECT_EQ(step->placements, (std::vector<Candidate>{{1, 1}}));
    EXPECT_EQ(step->reasons, (std::vector<Candidate>{{1, 1},
                                                     {geometry::Cell(1, 4), 1},
                                                     {geometry::Cell(2, 7), 1},
                                                     {geometry::Cell(6, 0), 1},
                                                     {geometry::Cell(7, 2), 1}}))
        << "i5 left out";
}

TEST(Techniques, ANakedSingleReadsItsCandidateAndTheValuesThatRuleOutTheRest)
{
    Grid grid;
    // Round e5: 1 to 4 on row 5, 5 and 6 in column e, 7 and 8 in its box.
    const int e5 = geometry::Cell(4, 4);
    for (int column = 0; column < 4; column++)
    {
        grid.Place(geometry::Cell(4, column), column + 1);
    }
    grid.Place(geometry::Cell(0, 4), 5);
    grid.Place(geometry::Cell(1, 4), 6);
    grid.Place(geometry::Cell(3, 3), 7);
    grid.Place(geometry::Cell(5, 5), 8);
    std::optional<Step> step = techniques::NakedSingle(grid);
    ASSERT_TRUE(step);
    EXPECT_EQ(step->placements, (std::vector<Candidate>{{e5, 9}}));
    std::vector<Candidate> expected{{e5, 9}};
    for (int column = 0; column < 4; column++)
    {
        expected.push_back(Candidate{geometry::Cell(4, column), column + 1});
    }
    expected.push_back(Candidate{geometry::Cell(0, 4), 5});
    expected.push_back(Candidate{geometry::Cell(1, 4), 6});
    expected.push_back(Candidate{geometry::Cell(3, 3), 7});
    expected.push_back(Candidate{geometry::Cell(5, 5), 8});
    EXPECT_EQ(step->reasons, expected) << "its candidate, then a value for each digit it cannot be";
}

TEST(Techniques, RemovesNamesEachDigitsSquaresInOrder)
{
    EXPECT_EQ(Removes({}), "removes nothing");
    std::vector<Candidate> one{{geometry::Cell(3, 8), 3}, {geometry::Cell(3, 0), 3}};
    EXPECT_EQ(Removes(one), "removes 3 from a6, i6");
    std::vector<Candidate> three{{40, 7}, {geometry::Cell(3, 8), 3}, {0, 1}, {geometry::Cell(3, 0), 3}};
    EXPECT_EQ(Removes(three), "removes 1 from a9, 3 from a6, i6 and 7 from e5");
    EXPECT_EQ(TechniqueName(Technique::kXYZWing), "XYZ-wing");
    EXPECT_EQ((Candidate{40, 5}.Text()), "e5=5");
}

TEST(Techniques, EveryTechniqueHasANameOfItsOwn)
{
    // The steps list and its descriptions name each technique: no two may read alike, and none may be the fallback.
    std::set<std::string_view> names;
    for (const Technique technique : kTechniques)
    {
        const std::string_view name = TechniqueName(technique);
        EXPECT_FALSE(name.empty());
        EXPECT_NE(name, "Unknown technique");
        EXPECT_TRUE(names.insert(name).second) << name << " names two techniques";
    }
    EXPECT_EQ(names.size(), kTechniques.size());
    EXPECT_EQ(TechniqueName(kTechniques.front()), "Naked single");
    EXPECT_EQ(TechniqueName(Technique::kSimpleColouring), "Simple colouring");
    EXPECT_EQ(TechniqueName(kTechniques.back()), "Contradiction chain");
}

TEST(Techniques, TheCombinationsAreEveryChoiceOnceInOrder)
{
    EXPECT_EQ(techniques::Combinations(std::vector<int>{1, 2, 3, 4}, 2),
              (std::vector<std::vector<int>>{{1, 2}, {1, 3}, {1, 4}, {2, 3}, {2, 4}, {3, 4}}));
    EXPECT_TRUE(techniques::Combinations(std::vector<int>{1, 2}, 3).empty());
}

} // namespace
} // namespace sudoku
