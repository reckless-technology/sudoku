#pragma once
/// @file puzzles.h The puzzles in the repository's puzzles/ folder, for the suites that solve them.

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace sudoku::test
{

/** @return The puzzles' folder, which the build names (SUDOKU_PUZZLES_DIR). */
inline std::filesystem::path PuzzlesFolder()
{
    return std::filesystem::path(SUDOKU_PUZZLES_DIR);
}

/** @return The puzzles' file names, in order. */
inline std::vector<std::string> PuzzleNames()
{
    std::vector<std::string> names;
    for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(PuzzlesFolder()))
    {
        if (entry.is_regular_file())
        {
            names.push_back(entry.path().filename().string());
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

/**
 * @param name A puzzle's file name.
 * @return The name as a test's name: "xwing1.sdk" as "xwing1_sdk".
 */
inline std::string TestName(std::string name)
{
    std::replace(name.begin(), name.end(), '.', '_');
    return name;
}

} // namespace sudoku::test
