// New puzzles from the puzzle bank, with no window: the controller choosing one of a band at random from the
// repository's sample of the bank (the same format), undone as an opened file is, and saying why when there is no bank
// or a record is damaged; the
// menu's New puzzle choices; and the bank found where the build and the installers lay it out beside the executable.

#include "app/controller.h"
#include "app/menus.h"
#include "app/puzzle_bank_folder.h"
#include "solver/puzzle_bank.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

namespace sudoku::app
{
namespace
{

/** @return The repository's sample of the bank. */
std::filesystem::path SampleBank()
{
    return std::filesystem::path(SUDOKU_PUZZLES_DIR) / "sudoku_exchange";
}

/** A folder of its own under the system's temporary folder, emptied before and removed after each test. */
class TemporaryFolder : public testing::Test
{
  protected:
    void SetUp() override
    {
        folder_ = std::filesystem::temp_directory_path() /
                  ("sudoku_app_bank_" + std::string(testing::UnitTest::GetInstance()->current_test_info()->name()));
        std::filesystem::remove_all(folder_);
        std::filesystem::create_directories(folder_);
    }

    void TearDown() override
    {
        std::error_code error;
        std::filesystem::remove_all(folder_, error);
    }

    /**
     * A bank of one record a band, written.
     * @param folder Where.
     * @param record The record, its line feed included.
     */
    static void WriteBank(const std::filesystem::path &folder, const std::string &record)
    {
        std::filesystem::create_directories(folder);
        for (const puzzle_bank::Band band : puzzle_bank::kBands)
        {
            std::ofstream(folder / puzzle_bank::FileName(band), std::ios::binary) << record;
        }
    }

    std::filesystem::path folder_; ///< The folder.
};

/** A record of the bank's format: a puzzle with one given, a1's 5. */
const std::string kRecord = "0123456789ab " + std::string(72, '0') + "500000000" + "  1.2\n";

TEST(NewPuzzle, IsOneOfItsBandsChosenAtRandomAndSaysWhichAndHowHard)
{
    Controller controller({}, SampleBank());
    ASSERT_TRUE(controller.HasPuzzleBank());
    std::set<Values> hard;
    for (std::size_t index = 0; index < puzzle_bank::Count(SampleBank(), puzzle_bank::Band::kHard); ++index)
    {
        hard.insert(puzzle_bank::Read(SampleBank(), puzzle_bank::Band::kHard, index)->givens);
    }
    std::set<Values> chosen;
    for (int pick = 0; pick < 20; ++pick)
    {
        ASSERT_TRUE(controller.NewPuzzle(puzzle_bank::Band::kHard));
        const Values givens = controller.game().givens();
        EXPECT_TRUE(hard.contains(givens)) << "a hard puzzle of the bank";
        chosen.insert(givens);
        EXPECT_TRUE(controller.file_name().starts_with("Hard ")) << controller.file_name();
        EXPECT_TRUE(controller.Message().starts_with("Hard puzzle ")) << controller.Message();
        EXPECT_NE(controller.Message().find(", rated "), std::string::npos) << controller.Message();
        EXPECT_NE(controller.Message().find("one of 1000 in the Sudoku Exchange puzzle bank."), std::string::npos)
            << controller.Message();
        EXPECT_TRUE(controller.CanRun());
    }
    EXPECT_GT(chosen.size(), 1u) << "twenty picks of a thousand are not all one";
    ASSERT_TRUE(controller.NewPuzzle(puzzle_bank::Band::kEasy));
    EXPECT_TRUE(controller.file_name().starts_with("Easy "));
}

TEST(NewPuzzle, WithoutTheBankLeavesTheBoardAndSaysWhy)
{
    for (const std::filesystem::path &folder : {std::filesystem::path(), std::filesystem::path(SUDOKU_PUZZLES_DIR)})
    {
        Controller controller({}, folder);
        EXPECT_FALSE(controller.HasPuzzleBank()) << folder;
        controller.Enter(0, 7);
        EXPECT_FALSE(controller.NewPuzzle(puzzle_bank::Band::kMedium));
        EXPECT_EQ(controller.game().givens()[0], 7) << "the board as it was";
        EXPECT_NE(controller.Message().find("the puzzle bank is not installed"), std::string::npos)
            << controller.Message();
    }
}

TEST_F(TemporaryFolder, ADamagedRecordIsSaidAndLeavesTheBoard)
{
    std::string damaged = kRecord;
    damaged[40]         = 'x';
    WriteBank(folder_, damaged);
    Controller controller({}, folder_);
    ASSERT_TRUE(controller.HasPuzzleBank()) << "whole records, though one is not a puzzle";
    EXPECT_FALSE(controller.NewPuzzle(puzzle_bank::Band::kDiabolical));
    EXPECT_EQ(controller.Message(), "No new puzzle: the puzzle bank's diabolical.txt is damaged at record 1.");
    EXPECT_EQ(controller.game().FilledCount(), 0);
}

TEST_F(TemporaryFolder, ARecordsPuzzleIsSetAsTheGivens)
{
    WriteBank(folder_, kRecord);
    Controller controller({}, folder_);
    controller.Enter(0, 7);
    ASSERT_TRUE(controller.NewPuzzle(puzzle_bank::Band::kMedium));
    EXPECT_EQ(controller.game().givens()[72], 5);
    EXPECT_EQ(controller.game().FilledCount(), 1);
    EXPECT_EQ(controller.file_name(), "Medium 0123456789ab");
    EXPECT_EQ(controller.Message(),
              "Medium puzzle 0123456789ab, rated 1.2, one of 1 in the Sudoku Exchange puzzle bank.");
    // Undone as an opened file is: the board before it back.
    controller.Undo();
    EXPECT_EQ(controller.game().givens()[0], 7);
    EXPECT_EQ(controller.game().FilledCount(), 1);
}

TEST(NewPuzzle, TheMenuOffersEachBandWhenThereIsABank)
{
    for (const bool has_bank : {false, true})
    {
        Controller      controller({}, has_bank ? SampleBank() : std::filesystem::path());
        const MenuState state = MenuStateOf(controller, WindowSizeMenu{});
        EXPECT_EQ(state.has_puzzle_bank, has_bank);
        const reckless::app::MenuBar    bar  = BuildMenuBar(state);
        const reckless::app::MenuEntry &file = bar.menus.front();
        ASSERT_EQ(file.title, "File");
        const reckless::app::MenuEntry &new_puzzle = file.submenu.front();
        EXPECT_EQ(new_puzzle.title, "New puzzle");
        ASSERT_EQ(new_puzzle.submenu.size(), puzzle_bank::kBands.size());
        for (std::size_t index = 0; index < puzzle_bank::kBands.size(); ++index)
        {
            const reckless::app::MenuEntry &item = new_puzzle.submenu[index];
            EXPECT_EQ(item.title, puzzle_bank::BandName(puzzle_bank::kBands[index]));
            EXPECT_EQ(item.id, kNewEasy + static_cast<int>(index));
            EXPECT_EQ(item.enabled, has_bank);
        }
    }
    Controller controller({}, SampleBank());
    for (const int command : {kNewEasy, kNewMedium, kNewHard, kNewDiabolical})
    {
        RunCommand(command, controller, CommandHandlers{}, WindowSizeMenu{});
        const std::string band(
            puzzle_bank::BandName(puzzle_bank::kBands[static_cast<std::size_t>(command - kNewEasy)]));
        EXPECT_TRUE(controller.file_name().starts_with(band + " ")) << controller.file_name();
    }
}

TEST_F(TemporaryFolder, TheBankIsFoundWhereEachPlatformLaysItOut)
{
    const std::filesystem::path bin = folder_ / "bin";
    std::filesystem::create_directories(bin);
    EXPECT_TRUE(PuzzleBankBeside(bin).empty()) << "none yet";
    EXPECT_TRUE(PuzzleBankBeside({}).empty());
    // The Debian package: share/reckless-sudoku/puzzles beside bin.
    WriteBank(folder_ / "share" / "reckless-sudoku" / "puzzles", kRecord);
    EXPECT_EQ(PuzzleBankBeside(bin), folder_ / "share" / "reckless-sudoku" / "puzzles");
    // macOS: the bundle's Resources/puzzles beside Contents/MacOS.
    WriteBank(folder_ / "Resources" / "puzzles", kRecord);
    EXPECT_EQ(PuzzleBankBeside(bin), folder_ / "Resources" / "puzzles");
    // Windows, the Linux archive and a build tree: puzzles beside the executable, looked for first.
    WriteBank(bin / "puzzles", kRecord);
    EXPECT_EQ(PuzzleBankBeside(bin), bin / "puzzles");
    // A folder short of a band is no bank.
    std::filesystem::remove(bin / "puzzles" / "easy.txt");
    EXPECT_EQ(PuzzleBankBeside(bin), folder_ / "Resources" / "puzzles");
}

TEST(NewPuzzle, TheExecutablesFolderIsTheSuites)
{
    const std::filesystem::path folder = ExecutableFolder();
    ASSERT_FALSE(folder.empty());
    EXPECT_TRUE(std::filesystem::is_directory(folder));
    bool has_suite = false;
    for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(folder))
    {
        has_suite = has_suite || entry.path().stem() == "sudoku_app_test";
    }
    EXPECT_TRUE(has_suite) << folder;
}

} // namespace
} // namespace sudoku::app
