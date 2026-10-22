#pragma once
/// @file puzzles.h The puzzles the suites solve: those in the repository's puzzles/ folder, each named for the
/// technique it calls for, and the Sudoku Exchange sample in puzzles/sudoku_exchange, rated and sorted into four bands
/// - with the check every suite holds a solve to: every value placed the solution's, every candidate struck not the
/// solution's.

#include "solver/brute_force.h"
#include "solver/grid.h"
#include "solver/solver.h"
#include "solver/step.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace sudoku::test
{

/** @return The puzzles' folder, which the build names (SUDOKU_PUZZLES_DIR). */
inline std::filesystem::path PuzzlesFolder()
{
    return std::filesystem::path(SUDOKU_PUZZLES_DIR);
}

/** @return The puzzles' file names, in order: the files in the folder itself, not in the folders under it. */
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

/** The Sudoku Exchange sample's bands, its files' names, easiest first. */
inline constexpr std::array<std::string_view, 4> kRatedBands{"easy", "medium", "hard", "diabolical"};

/** A record of the Sudoku Exchange sample (puzzles/sudoku_exchange/README.md). */
struct RatedPuzzle
{
    std::string band;        ///< Its file: "easy", "medium", "hard" or "diabolical".
    std::string record;      ///< The record as the file holds it, without its line feed.
    std::string id;          ///< The 12 hex digits the bank orders its records by.
    std::string digits;      ///< The 81 squares, row by row from the top left, '0' for an empty one.
    double      rating{0.0}; ///< Sukaku Explainer's rating.

    /** @return The squares' values. */
    Values Givens() const
    {
        Values values{};
        for (std::size_t index = 0; index < values.size() && index < digits.size(); ++index)
        {
            values[index] = digits[index] - '0';
        }
        return values;
    }
};

/**
 * @param band A band: one of kRatedBands.
 * @return Its records, in the file's order, each split into its fields where it is long enough to hold them (the
 * suite holds the records to their format).
 */
inline std::vector<RatedPuzzle> RatedPuzzles(std::string_view band)
{
    std::ifstream            file(PuzzlesFolder() / "sudoku_exchange" / (std::string(band) + ".txt"), std::ios::binary);
    std::vector<RatedPuzzle> puzzles;
    std::string              line;
    while (std::getline(file, line))
    {
        RatedPuzzle puzzle;
        puzzle.band   = band;
        puzzle.record = line;
        if (line.size() >= 99)
        {
            puzzle.id     = line.substr(0, 12);
            puzzle.digits = line.substr(13, 81);
            puzzle.rating = std::strtod(line.c_str() + 95, nullptr);
        }
        puzzles.push_back(std::move(puzzle));
    }
    return puzzles;
}

/** @return Every rated puzzle, the easiest band first. */
inline std::vector<RatedPuzzle> AllRatedPuzzles()
{
    std::vector<RatedPuzzle> all;
    for (std::string_view band : kRatedBands)
    {
        std::vector<RatedPuzzle> puzzles = RatedPuzzles(band);
        all.insert(all.end(), puzzles.begin(), puzzles.end());
    }
    return all;
}

/**
 * A puzzle solved by deduction, with every step held to its one solution: each value it places the solution's, each
 * candidate it strikes not, each with words and squares to show; and the board finished as the solution.
 * @param givens The puzzle.
 * @param name Its name, for the failures' messages.
 * @return The hardest technique it took - the latest in the solver's order; none for a puzzle with nothing to deduce.
 */
inline std::optional<Technique> ExpectSolvedByDeduction(std::span<const int> givens, const std::string &name)
{
    Values solution{};
    EXPECT_EQ(CountSolutions(givens, &solution), SolutionCount::kOne) << name << ": one solution";
    Grid                     grid = Grid::FromValues(givens);
    Solver                   solver(grid);
    std::optional<Technique> hardest;
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
        EXPECT_NE(step->description.find_first_not_of(" \t\r\n"), std::string::npos) << name;
        EXPECT_FALSE(step->pattern.empty()) << name << ": " << step->description;
        EXPECT_FALSE(step->reasons.empty()) << name << ": " << step->description;
        if (!hardest || step->technique > *hardest)
        {
            hardest = step->technique;
        }
    }
    EXPECT_TRUE(grid.IsComplete()) << name << ": stuck with "
                                   << std::count(grid.values().begin(), grid.values().end(), 0) << " squares empty";
    EXPECT_EQ(grid.values(), solution) << name;
    return hardest;
}

} // namespace sudoku::test
