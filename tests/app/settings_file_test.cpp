// The choices kept between launches (app/settings_file.h): read back as written, the defaults for what is missing or
// makes no sense, and the keeper writing only when the choices change.

#include "app/settings_file.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace sudoku::app
{
namespace
{

/** A settings file in the system's temporary folder, removed with its folder when the test ends. */
class SettingsFileTest : public ::testing::Test
{
  protected:
    void TearDown() override
    {
        std::filesystem::remove_all(folder_);
    }

    /** @param text The file's contents, written. */
    void WriteText(const std::string &text)
    {
        std::filesystem::create_directories(folder_);
        std::ofstream file(file_, std::ios::binary);
        file << text;
    }

    /** A folder of the test's own, so the suites can run side by side. */
    std::filesystem::path folder_{
        std::filesystem::temp_directory_path() /
        (std::string("sudoku-settings-") + ::testing::UnitTest::GetInstance()->current_test_info()->name())};
    std::filesystem::path file_{folder_ / "nested" / "settings.json"};
};

TEST_F(SettingsFileTest, AMissingFileGivesTheDefaults)
{
    EXPECT_EQ(ReadSettings(file_), Settings{});
    EXPECT_EQ(ReadSettings({}), Settings{});
    EXPECT_FALSE(WriteSettings({}, Settings{}));
}

TEST_F(SettingsFileTest, TheChoicesReadBackAsWritten)
{
    const Settings settings{false, 125};
    ASSERT_TRUE(WriteSettings(file_, settings)); // its folder made first
    EXPECT_EQ(ReadSettings(file_), settings);
}

TEST_F(SettingsFileTest, WhatMakesNoSenseIsTheDefault)
{
    file_ = folder_ / "settings.json";
    WriteText(R"({"pencil_marks": "yes", "window_percent": 300})");
    EXPECT_EQ(ReadSettings(file_), Settings{});
    WriteText(R"({"window_percent": 75})");
    EXPECT_EQ(ReadSettings(file_), (Settings{false, 75})) << "the marks default to off";
    WriteText("not json");
    EXPECT_EQ(ReadSettings(file_), Settings{});
    WriteText("[1, 2]");
    EXPECT_EQ(ReadSettings(file_), Settings{});
}

TEST_F(SettingsFileTest, TheKeeperWritesOnlyWhenTheChoicesChange)
{
    SettingsKeeper keeper(file_);
    const Settings read = keeper.Read();
    EXPECT_EQ(read, Settings{});
    EXPECT_TRUE(keeper.Changed(read)) << "the first call: the menu's ticks set";
    EXPECT_EQ(keeper.writes(), 0) << "nothing written for what was read";
    EXPECT_FALSE(keeper.Changed(read));
    EXPECT_TRUE(keeper.Changed(Settings{true, 100}));
    EXPECT_EQ(keeper.writes(), 1);
    EXPECT_EQ(ReadSettings(file_), (Settings{true, 100}));
    EXPECT_FALSE(keeper.Changed(Settings{true, 100}));
    EXPECT_EQ(keeper.writes(), 1);

    SettingsKeeper none({}); // a launch's check: nothing kept
    none.Read();
    EXPECT_TRUE(none.Changed(Settings{false, 50}));
    EXPECT_EQ(none.writes(), 0);
}

TEST(SettingsFile, LivesInTheSuitesFolder)
{
    const std::filesystem::path file = DefaultSettingsFile();
    if (!file.empty())
    {
        EXPECT_EQ(file.filename(), "settings.json");
        EXPECT_EQ(file.parent_path().filename(), "sudoku");
    }
}

} // namespace
} // namespace sudoku::app
