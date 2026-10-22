/// @file game_test.cpp The board a person works on: nothing deduced from a value outlives it, clashing givens found
/// before the solver runs, every change undone, a puzzle's solutions counted.

#include "puzzles.h"

#include "solver/brute_force.h"
#include "solver/digits.h"
#include "solver/game.h"
#include "solver/puzzle_file.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace sudoku
{
namespace
{

/**
 * @param game A game.
 * @return How many squares have a value, given or deduced, counted square by square.
 */
int CountValues(const Game &game)
{
    int count = 0;
    for (int cell = 0; cell < geometry::kCellCount; cell++)
    {
        if (game.Value(cell) != 0)
        {
            count++;
        }
    }
    return count;
}

TEST(Game, ClearingAGivenGivesBackTheCandidatesItStruck)
{
    Game game;
    game.SetGiven(0, 5);
    EXPECT_FALSE(digits::Contains(game.Candidates(1), 5));
    game.Clue();
    game.SetGiven(0, 0);
    for (int cell = 1; cell < geometry::kCellCount; cell++)
    {
        EXPECT_EQ(game.Candidates(cell), digits::kAll) << geometry::Name(cell);
    }
    EXPECT_TRUE(game.steps().empty());
}

TEST(Game, ChangingAGivenDropsWhatWasDeducedFromTheOldOne)
{
    Game game;
    game.Load(puzzle_file::Read(test::PuzzlesFolder() / "easy1.sud"));
    EXPECT_FALSE(game.Solve().empty());
    EXPECT_TRUE(game.IsSolved());
    int empty = 0;
    while (game.IsGiven(empty))
    {
        empty++;
    }
    int other = game.Value(empty) == 1 ? 2 : 1;
    game.SetGiven(empty, other);
    EXPECT_TRUE(game.steps().empty());
    for (int cell = 0; cell < geometry::kCellCount; cell++)
    {
        if (!game.IsGiven(cell))
        {
            EXPECT_EQ(game.Value(cell), 0) << geometry::Name(cell);
        }
    }
}

TEST(Game, ClashingGivensAreNamedAndTheSolverWaitsForThem)
{
    Game game;
    game.SetGiven(geometry::Cell(0, 0), 5);
    game.SetGiven(geometry::Cell(0, 8), 5);
    EXPECT_EQ(game.clashes(), (std::vector<int>{geometry::Cell(0, 0), geometry::Cell(0, 8)}));
    EXPECT_EQ(game.Contradiction(), "5 is twice in row 9 (a9, i9)");
    EXPECT_TRUE(game.Clue().empty());
    EXPECT_TRUE(game.Solve().empty());
    EXPECT_EQ(game.Candidates(geometry::Cell(4, 4)), 0);
    game.SetGiven(geometry::Cell(0, 8), 0);
    EXPECT_TRUE(game.clashes().empty());
    EXPECT_EQ(game.Contradiction(), std::nullopt);
}

TEST(Game, GivensThatClashWithNothingButLeaveNoWayAreNamedToo)
{
    Game game;
    // Row 9 holds 1 to 8 and i8 is 9: i9 can be nothing.
    for (int column = 0; column < 8; column++)
    {
        game.SetGiven(geometry::Cell(0, column), column + 1);
    }
    game.SetGiven(geometry::Cell(1, 8), 9);
    EXPECT_TRUE(game.clashes().empty());
    EXPECT_EQ(game.Contradiction(), "i9 has no digit left");
    EXPECT_EQ(game.CountSolutions(), SolutionCount::kNone);
}

TEST(Game, UndoTakesBackEachChangeInTurn)
{
    Game game;
    EXPECT_FALSE(game.CanUndo());
    game.Load(puzzle_file::Read(test::PuzzlesFolder() / "medium1.sud"));
    Values            puzzle = game.givens();
    std::vector<Step> clue   = game.Clue();
    ASSERT_FALSE(clue.empty());
    ASSERT_TRUE(clue.back().IsPlacement());
    Candidate placed = clue.back().placements[0];
    EXPECT_EQ(game.Value(placed.cell), placed.digit);
    game.SetGiven(placed.cell, 0);
    EXPECT_EQ(game.Value(placed.cell), 0);
    EXPECT_FALSE(game.SetGiven(placed.cell, 0)); // nothing to change: no undo for it

    EXPECT_TRUE(game.Undo()); // the clearing
    EXPECT_EQ(game.Value(placed.cell), placed.digit);
    EXPECT_EQ(game.steps().size(), clue.size());
    EXPECT_TRUE(game.Undo()); // the clue
    EXPECT_EQ(game.Value(placed.cell), 0);
    EXPECT_TRUE(game.steps().empty());
    EXPECT_TRUE(game.Undo()); // the load
    EXPECT_TRUE(std::all_of(game.givens().begin(), game.givens().end(), [](int value) { return value == 0; }));
    EXPECT_FALSE(game.Undo());
    EXPECT_NE(game.givens(), puzzle);
}

TEST(Game, ACluePlacesOneValueAndShowsTheStepsToIt)
{
    Game game;
    game.Load(puzzle_file::Read(test::PuzzlesFolder() / "xwing1.sdk"));
    int               before = CountValues(game);
    std::vector<Step> steps  = game.Clue();
    ASSERT_FALSE(steps.empty());
    EXPECT_TRUE(steps.back().IsPlacement());
    EXPECT_TRUE(std::none_of(steps.begin(), steps.end() - 1, [](const Step &step) { return step.IsPlacement(); }));
    EXPECT_EQ(CountValues(game), before + 1);
    EXPECT_EQ(game.FilledCount(), before + 1);
}

TEST(Game, SolutionsAreCountedNoneOneOrMany)
{
    Values any{};
    EXPECT_EQ(CountSolutions(Values{}, &any), SolutionCount::kMany);
    EXPECT_TRUE(std::none_of(any.begin(), any.end(), [](int value) { return value == 0; }));
    EXPECT_EQ(Grid::FromValues(any).FindContradiction(), std::nullopt);
    Values puzzle = puzzle_file::Read(test::PuzzlesFolder() / "hard1.sud");
    Values solution{};
    EXPECT_EQ(CountSolutions(puzzle, &solution), SolutionCount::kOne);
    EXPECT_TRUE(std::none_of(solution.begin(), solution.end(), [](int value) { return value == 0; }));
    auto empty    = static_cast<std::size_t>(std::find(puzzle.begin(), puzzle.end(), 0) - puzzle.begin());
    puzzle[empty] = solution[empty] % 9 + 1; // a digit the one solution rules out
    EXPECT_EQ(CountSolutions(puzzle), SolutionCount::kNone);
}

TEST(Game, ClearingTheBoardEmptiesEverySquareAndIsUndone)
{
    Game game;
    game.Load(puzzle_file::Read(test::PuzzlesFolder() / "easy1.sud"));
    Values puzzle = game.givens();
    game.Clear();
    EXPECT_EQ(game.FilledCount(), 0);
    EXPECT_EQ(game.givens(), Values{});
    EXPECT_TRUE(game.Undo());
    EXPECT_EQ(game.givens(), puzzle);
}

TEST(Game, AValueOffTheBoardOrOutsideZeroToNineIsRefused)
{
    Game game;
    EXPECT_THROW(game.SetGiven(0, 10), std::out_of_range);
    EXPECT_THROW(game.SetGiven(0, -1), std::out_of_range);
    EXPECT_THROW(game.SetGiven(-1, 5), std::out_of_range);
    EXPECT_THROW(game.SetGiven(geometry::kCellCount, 5), std::out_of_range);
    EXPECT_THROW(game.Load(std::vector<int>(80, 0)), std::invalid_argument);
    Values ten{};
    ten[40] = 10;
    EXPECT_THROW(game.Load(ten), std::invalid_argument);
    EXPECT_FALSE(game.CanUndo());
    EXPECT_THROW(Grid::FromValues(std::vector<int>(82, 0)), std::invalid_argument);
    EXPECT_THROW(Grid::FromValues(ten), std::invalid_argument);
    EXPECT_THROW(CountSolutions(ten), std::invalid_argument);
}

} // namespace
} // namespace sudoku
