/// @file puzzle_bank_test.cpp The puzzle bank's reader, on the repository's sample of it (puzzles/sudoku_exchange, the
/// first records of each of the bank's files, in its format): each band counted, any record read by its index alone and
/// read as the sample's line says, and a folder or a record not in the format refused.

#include "puzzles.h"

#include "solver/puzzle_bank.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace sudoku
{
namespace
{

/** @return The sample's folder. */
std::filesystem::path SampleFolder()
{
    return test::PuzzlesFolder() / "sudoku_exchange";
}

/** A folder of its own under the system's temporary folder, emptied before and removed after each test. */
class PuzzleBankFolder : public testing::Test
{
  protected:
    void SetUp() override
    {
        folder_ = std::filesystem::temp_directory_path() /
                  ("sudoku_bank_" + std::string(testing::UnitTest::GetInstance()->current_test_info()->name()));
        std::filesystem::remove_all(folder_);
        std::filesystem::create_directories(folder_);
    }

    void TearDown() override
    {
        std::error_code error;
        std::filesystem::remove_all(folder_, error);
    }

    /**
     * A band's file, written.
     * @param band The band.
     * @param text What it holds.
     */
    void WriteBand(puzzle_bank::Band band, const std::string &text) const
    {
        std::ofstream(folder_ / puzzle_bank::FileName(band), std::ios::binary) << text;
    }

    std::filesystem::path folder_; ///< The folder.
};

TEST(PuzzleBank, NamesItsBandsAndTheirFiles)
{
    EXPECT_EQ(puzzle_bank::BandName(puzzle_bank::Band::kEasy), "Easy");
    EXPECT_EQ(puzzle_bank::BandName(puzzle_bank::Band::kMedium), "Medium");
    EXPECT_EQ(puzzle_bank::BandName(puzzle_bank::Band::kHard), "Hard");
    EXPECT_EQ(puzzle_bank::BandName(puzzle_bank::Band::kDiabolical), "Diabolical");
    EXPECT_EQ(puzzle_bank::FileName(puzzle_bank::Band::kDiabolical), "diabolical.txt");
    // The bands in the order of the sample's, which the suites read by name.
    for (std::size_t index = 0; index < puzzle_bank::kBands.size(); ++index)
    {
        EXPECT_EQ(puzzle_bank::FileName(puzzle_bank::kBands[index]), std::string(test::kRatedBands[index]) + ".txt");
    }
}

TEST(PuzzleBank, CountsEachBandAndReadsAnyRecordAsItsLineSays)
{
    EXPECT_TRUE(puzzle_bank::IsBank(SampleFolder()));
    for (std::size_t band_index = 0; band_index < puzzle_bank::kBands.size(); ++band_index)
    {
        const puzzle_bank::Band              band  = puzzle_bank::kBands[band_index];
        const std::vector<test::RatedPuzzle> lines = test::RatedPuzzles(test::kRatedBands[band_index]);
        ASSERT_EQ(puzzle_bank::Count(SampleFolder(), band), lines.size());
        // The first, the last and one between, each by its index alone.
        for (const std::size_t index : {std::size_t{0}, lines.size() / 2, lines.size() - 1})
        {
            const std::optional<puzzle_bank::Puzzle> puzzle = puzzle_bank::Read(SampleFolder(), band, index);
            ASSERT_TRUE(puzzle.has_value()) << lines[index].record;
            EXPECT_EQ(puzzle->band, band);
            EXPECT_EQ(puzzle->index, index);
            EXPECT_EQ(puzzle->id, lines[index].id);
            EXPECT_EQ(puzzle->givens, lines[index].Givens());
            EXPECT_DOUBLE_EQ(puzzle->rating, lines[index].rating);
        }
        EXPECT_FALSE(puzzle_bank::Read(SampleFolder(), band, lines.size())) << "past the end";
    }
}

TEST(PuzzleBank, ReadsARecordsFields)
{
    const std::string                        record = "0123456789ab " + std::string(80, '0') + "7 10.4";
    const std::optional<puzzle_bank::Puzzle> puzzle = puzzle_bank::Parse(record, puzzle_bank::Band::kDiabolical, 3);
    ASSERT_TRUE(puzzle.has_value());
    EXPECT_EQ(puzzle->id, "0123456789ab");
    EXPECT_EQ(puzzle->givens[80], 7);
    EXPECT_EQ(puzzle->givens[0], 0);
    EXPECT_DOUBLE_EQ(puzzle->rating, 10.4);
    EXPECT_EQ(puzzle->index, 3u);
}

TEST(PuzzleBank, RefusesARecordNotInItsFormat)
{
    const std::string good = "0123456789ab " + std::string(81, '1') + "  1.2";
    ASSERT_TRUE(puzzle_bank::Parse(good, puzzle_bank::Band::kEasy, 0));
    std::vector<std::string> bad(6, good);
    bad[0].pop_back(); // short
    bad[1][3]  = 'G';  // not hex
    bad[2][12] = '-';  // no space after the id
    bad[3][40] = '.';  // a square not a digit
    bad[4][94] = '1';  // no space before the rating
    bad[5][97] = 'x';  // not a number
    for (const std::string &record : bad)
    {
        EXPECT_FALSE(puzzle_bank::Parse(record, puzzle_bank::Band::kEasy, 0)) << record;
    }
}

TEST_F(PuzzleBankFolder, IsNoBankWithoutEveryBandOrWithAPartRecord)
{
    EXPECT_FALSE(puzzle_bank::IsBank(folder_)) << "empty";
    const std::string record = "0123456789ab " + std::string(81, '1') + "  1.2\n";
    for (const puzzle_bank::Band band : puzzle_bank::kBands)
    {
        WriteBand(band, record + record);
    }
    EXPECT_TRUE(puzzle_bank::IsBank(folder_));
    EXPECT_EQ(puzzle_bank::Count(folder_, puzzle_bank::Band::kHard), 2u);
    WriteBand(puzzle_bank::Band::kHard, record + "0123");
    EXPECT_EQ(puzzle_bank::Count(folder_, puzzle_bank::Band::kHard), 0u) << "not a whole number of records";
    EXPECT_FALSE(puzzle_bank::IsBank(folder_));
    // A record without its line feed: the right length, the wrong last byte.
    WriteBand(puzzle_bank::Band::kHard, record.substr(0, 99) + "x");
    EXPECT_EQ(puzzle_bank::Count(folder_, puzzle_bank::Band::kHard), 1u);
    EXPECT_FALSE(puzzle_bank::Read(folder_, puzzle_bank::Band::kHard, 0));
    std::filesystem::remove(folder_ / puzzle_bank::FileName(puzzle_bank::Band::kEasy));
    EXPECT_EQ(puzzle_bank::Count(folder_, puzzle_bank::Band::kEasy), 0u);
    EXPECT_FALSE(puzzle_bank::Read(folder_, puzzle_bank::Band::kEasy, 0));
}

} // namespace
} // namespace sudoku
