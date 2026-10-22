/// @file rated_puzzle_test.cpp The Sudoku Exchange sample (puzzles/sudoku_exchange): 2,250 puzzles another program made
/// and another rated, each solved by deduction with every step held to its one solution; the easy ones by singles
/// alone and the medium ones by nothing past pairs and the box and line intersections; and the harder the band, the
/// later in the solver's order the techniques its puzzles take.

#include "puzzles.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace sudoku
{
namespace
{

/** A band of the sample: its ratings, as the bank sorts them, and how many of its records the sample holds. */
struct Band
{
    std::string_view name;    ///< Its file's name.
    double           lowest;  ///< The lowest rating in it.
    double           below;   ///< Every rating in it is below this.
    std::size_t      records; ///< The records the sample holds.
};

/** The bands, easiest first (puzzles/sudoku_exchange/README.md). */
const std::vector<Band> kBands{
    {"easy", 0.0, 1.5, 250}, {"medium", 1.5, 2.5, 500}, {"hard", 2.5, 5.0, 1000}, {"diabolical", 5.0, 100.0, 500}};

TEST(RatedPuzzles, TheSampleIsWhatItsReadmeSays)
{
    std::set<std::string> ids;
    std::set<std::string> digits;
    for (const Band &band : kBands)
    {
        const std::vector<test::RatedPuzzle> puzzles = test::RatedPuzzles(band.name);
        ASSERT_EQ(puzzles.size(), band.records) << band.name;
        std::string previous;
        for (const test::RatedPuzzle &puzzle : puzzles)
        {
            const std::string &record = puzzle.record;
            ASSERT_EQ(record.size(), 99u) << band.name << ": 100 bytes with the line feed: " << record;
            EXPECT_EQ(record[12], ' ') << record;
            EXPECT_EQ(record[94], ' ') << record;
            EXPECT_TRUE(std::all_of(puzzle.id.begin(), puzzle.id.end(), [](char c) {
                return std::isdigit(static_cast<unsigned char>(c)) || (c >= 'a' && c <= 'f');
            })) << record;
            EXPECT_TRUE(std::all_of(puzzle.digits.begin(), puzzle.digits.end(), [](char c) {
                return c >= '0' && c <= '9';
            })) << record;
            EXPECT_GE(puzzle.rating, band.lowest) << record;
            EXPECT_LT(puzzle.rating, band.below) << record;
            EXPECT_LT(previous, puzzle.id) << band.name << ": the bank's order, by id";
            previous = puzzle.id;
            EXPECT_TRUE(ids.insert(puzzle.id).second) << "an id twice: " << record;
            EXPECT_TRUE(digits.insert(puzzle.digits).second) << "a puzzle twice: " << record;
        }
    }
}

/** A rated puzzle. */
class EachRatedPuzzle : public testing::TestWithParam<test::RatedPuzzle>
{
};

TEST_P(EachRatedPuzzle, IsSolvedByDeductionAndEveryStepIsTrue)
{
    const test::RatedPuzzle       &puzzle  = GetParam();
    const std::string              name    = puzzle.band + " " + puzzle.id + " (" + std::to_string(puzzle.rating) + ")";
    const std::optional<Technique> hardest = test::ExpectSolvedByDeduction(puzzle.Givens(), name);
    ASSERT_TRUE(hardest.has_value()) << name << ": a puzzle with something to deduce";
    // The two easy bands, as their rater judged them, take nothing the solver tries late.
    if (puzzle.band == "easy")
    {
        EXPECT_LE(*hardest, Technique::kHiddenSingle) << name << " took " << TechniqueName(*hardest);
    }
    else if (puzzle.band == "medium")
    {
        EXPECT_LE(*hardest, Technique::kClaiming) << name << " took " << TechniqueName(*hardest);
    }
}

INSTANTIATE_TEST_SUITE_P(RatedPuzzles, EachRatedPuzzle, testing::ValuesIn(test::AllRatedPuzzles()),
                         [](const testing::TestParamInfo<test::RatedPuzzle> &info) {
                             return info.param.band + "_" + info.param.id;
                         });

TEST(RatedPuzzles, TheHarderTheBandTheLaterTheTechniquesItTakes)
{
    // The first 40 of each band: the hardest technique each takes, by its place in the solver's order, averaged; and
    // how many take a chain (an XY-chain or anything after it).
    std::map<std::string_view, double> mean;
    std::map<std::string_view, int>    chains;
    for (const Band &band : kBands)
    {
        std::vector<test::RatedPuzzle> puzzles = test::RatedPuzzles(band.name);
        puzzles.resize(std::min<std::size_t>(puzzles.size(), 40));
        int total = 0;
        for (const test::RatedPuzzle &puzzle : puzzles)
        {
            Grid grid    = Grid::FromValues(puzzle.Givens());
            int  hardest = 0;
            for (const Step &step : Solver(grid).Solve())
            {
                hardest = std::max(hardest, static_cast<int>(step.technique));
            }
            total += hardest;
            chains[band.name] += hardest >= static_cast<int>(Technique::kXYChain) ? 1 : 0;
        }
        mean[band.name] = static_cast<double>(total) / static_cast<double>(puzzles.size());
    }
    EXPECT_LT(mean["easy"], mean["medium"]);
    EXPECT_LT(mean["medium"], mean["hard"]);
    EXPECT_LT(mean["hard"], mean["diabolical"]);
    EXPECT_EQ(chains["easy"], 0);
    EXPECT_EQ(chains["medium"], 0);
    EXPECT_LT(chains["hard"], 20) << "most hard puzzles need no chain";
    EXPECT_GT(chains["diabolical"], 30) << "most diabolical ones do";
}

} // namespace
} // namespace sudoku
