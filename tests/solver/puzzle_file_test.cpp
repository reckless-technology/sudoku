/// @file puzzle_file_test.cpp Puzzles in files: 81 bytes in a .sud file, text in any other, each read back as written.

#include "puzzles.h"

#include "solver/puzzle_file.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

namespace sudoku
{
namespace
{

/** @return A puzzle to write and read. */
Values Puzzle()
{
    return puzzle_file::Parse("...13...5\n"
                              ".4....2..\n"
                              "8..9.....\n"
                              "....5.9..\n"
                              "..2...4..\n"
                              "..3.6....\n"
                              ".....3..6\n"
                              "..5....1.\n"
                              "7...28...\n");
}

/** A file in the temporary folder with a name no other has, deleted when this goes. */
class TemporaryFile
{
  public:
    /** @param extension The file's extension: ".sud". */
    explicit TemporaryFile(const std::string &extension)
    {
        std::random_device random;
        path_ = std::filesystem::temp_directory_path() /
                ("sudoku-" + std::to_string(random()) + "-" + std::to_string(random()) + extension);
    }

    TemporaryFile(const TemporaryFile &)            = delete;
    TemporaryFile &operator=(const TemporaryFile &) = delete;

    ~TemporaryFile()
    {
        std::error_code error;
        std::filesystem::remove(path_, error);
    }

    /** @return Where it is. */
    const std::filesystem::path &path() const noexcept
    {
        return path_;
    }

    /**
     * Writes bytes into it.
     * @param bytes The bytes.
     */
    void WriteBytes(const std::vector<char> &bytes) const
    {
        std::ofstream file(path_, std::ios::binary | std::ios::trunc);
        file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    }

  private:
    std::filesystem::path path_; ///< Where it is.
};

/** A file's extension. */
class EachExtension : public testing::TestWithParam<std::string>
{
};

TEST_P(EachExtension, APuzzleReadsBackAsItWasWritten)
{
    TemporaryFile file(GetParam());
    puzzle_file::Write(file.path(), Puzzle());
    EXPECT_EQ(puzzle_file::Read(file.path()), Puzzle());
    EXPECT_EQ(std::filesystem::file_size(file.path()), GetParam() == ".sud" ? 81u : 99u);
}

INSTANTIATE_TEST_SUITE_P(PuzzleFile, EachExtension, testing::Values(".sud", ".sdk", ".txt"),
                         [](const testing::TestParamInfo<std::string> &info) { return info.param.substr(1); });

TEST(PuzzleFile, TextTakesDotsOrZerosAndIgnoresEverythingElse)
{
    std::string zeros;
    for (char character : puzzle_file::Format(Puzzle()))
    {
        if (character == '\r')
        {
            continue;
        }
        zeros += character == '.'    ? std::string("0")
                 : character == '\n' ? std::string(" | ")
                                     : std::string(1, character);
    }
    EXPECT_EQ(puzzle_file::Parse(zeros), Puzzle());
    EXPECT_EQ(puzzle_file::Format(Puzzle()).substr(0, 1), ".");
}

TEST(PuzzleFile, AFileThatIsNotAPuzzleIsRefused)
{
    EXPECT_THROW(puzzle_file::Parse("123"), puzzle_file::Error);
    TemporaryFile file(".sud");
    file.WriteBytes(std::vector<char>(80, 0));
    EXPECT_THROW(puzzle_file::Read(file.path()), puzzle_file::Error);
    file.WriteBytes(std::vector<char>(81, 10));
    EXPECT_THROW(puzzle_file::Read(file.path()), puzzle_file::Error);
    file.WriteBytes(std::vector<char>(82, 0));
    EXPECT_THROW(puzzle_file::Read(file.path()), puzzle_file::Error);
}

TEST(PuzzleFile, ASudFileIsKnownInAnyCase)
{
    TemporaryFile file(".SUD");
    puzzle_file::Write(file.path(), Puzzle());
    EXPECT_EQ(std::filesystem::file_size(file.path()), 81u);
    EXPECT_EQ(puzzle_file::Read(file.path()), Puzzle());
}

TEST(PuzzleFile, AFileThatCannotBeReadOrWrittenIsAnError)
{
    TemporaryFile file(".sdk");
    EXPECT_THROW(puzzle_file::Read(file.path()), puzzle_file::Error);
    EXPECT_THROW(puzzle_file::Write(file.path() / "puzzle.sdk", Puzzle()), puzzle_file::Error);
}

TEST(PuzzleFile, TheSudFileAndTheSdkFileOfTheSamePuzzleAgree)
{
    EXPECT_EQ(puzzle_file::Read(test::PuzzlesFolder() / "xwing1.sdk"),
              puzzle_file::Read(test::PuzzlesFolder() / "xwing1.sud"));
}

} // namespace
} // namespace sudoku
