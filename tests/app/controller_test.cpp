// The window's state (app/controller.h) with no window: what each act does to the game and what it says, the square
// chosen and moved, the step lit, the board's picture, the files opened and saved, and the menus built from it all.

#include "app/controller.h"
#include "app/menus.h"
#include "solver/digits.h"
#include "solver/puzzle_file.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace sudoku::app
{
namespace
{

/** The X-wing puzzle (puzzles/xwing1.sdk). */
constexpr const char *kXWing = "...13...5.4....2..8..9.........5.9....2...4....3.6.........3..6..5....1.7...28...";

/** AI Escargot, which needs chains deeper than the solver follows. */
constexpr const char *kEscargot = "1....7.9..3..2...8..96..5....53..9...1..8...26....4...3......1..4......7..7...3..";

/**
 * Every value of a puzzle entered as a given, as a person types them.
 * @param controller The controller.
 * @param text The puzzle.
 */
void EnterPuzzle(Controller &controller, const char *text)
{
    const Values values = puzzle_file::Parse(text);
    for (int cell = 0; cell < geometry::kCellCount; ++cell)
    {
        if (values[static_cast<std::size_t>(cell)] != 0)
        {
            controller.Enter(cell, values[static_cast<std::size_t>(cell)]);
        }
    }
}

/** @return A file name in the system's temporary folder no other test uses. @param name Its name. */
std::filesystem::path Scratch(const std::string &name)
{
    return std::filesystem::temp_directory_path() / ("sudoku-controller-" + name);
}

TEST(Controller, StartsEmptyAndSaysHowToBegin)
{
    const Controller controller;
    EXPECT_EQ(controller.Headline(), "Enter a puzzle");
    EXPECT_EQ(controller.Message(), "Choose a square and type a digit, or open a puzzle.");
    EXPECT_EQ(controller.selected(), geometry::Cell(4, 4));
    EXPECT_TRUE(controller.CanRun());
    EXPECT_FALSE(controller.shown_step().has_value());
}

TEST(Controller, MovesTheChosenSquareAndHoldsItToTheBoard)
{
    Controller controller;
    controller.Select(geometry::Cell(0, 0));
    controller.MoveSelection(-1, -1);
    EXPECT_EQ(controller.selected(), geometry::Cell(0, 0));
    controller.MoveSelection(2, 3);
    EXPECT_EQ(controller.selected(), geometry::Cell(2, 3));
    controller.MoveSelection(20, 20);
    EXPECT_EQ(controller.selected(), geometry::Cell(8, 8));
    controller.Select(500);
    EXPECT_EQ(controller.selected(), geometry::kCellCount - 1);
    controller.Select(-3);
    EXPECT_EQ(controller.selected(), 0);
}

TEST(Controller, ACluePlacesAValueSaysItsStepAndLightsIt)
{
    Controller controller;
    EnterPuzzle(controller, kXWing);
    EXPECT_EQ(controller.Headline(), "20 of 81");
    controller.Clue();
    const Game &game = controller.game();
    ASSERT_FALSE(game.steps().empty());
    EXPECT_TRUE(game.steps().back().IsPlacement());
    EXPECT_EQ(controller.Message(), game.steps().back().description);
    ASSERT_TRUE(controller.shown_step().has_value());
    EXPECT_EQ(*controller.shown_step(), game.steps().size() - 1);
    EXPECT_EQ(controller.Headline(), "21 of 81");

    const BoardPicture picture = controller.Picture();
    const Candidate    placed  = game.steps().back().placements.front();
    EXPECT_EQ(picture.placed, std::vector<int>{placed.cell});
    EXPECT_EQ(picture.pattern, game.steps().back().pattern);
    EXPECT_EQ(picture.values[static_cast<std::size_t>(placed.cell)], placed.digit);
    EXPECT_FALSE(picture.is_given[static_cast<std::size_t>(placed.cell)]);
    // What the clue read, to ring: the candidate it placed, and the values that rule out the rest.
    ASSERT_FALSE(picture.reasons.empty());
    EXPECT_EQ(picture.reasons, game.steps().back().reasons);
    EXPECT_EQ(picture.reasons.front(), placed);
    EXPECT_FALSE(picture.IsReasonValue(placed.cell)) << "the value placed is washed green, not ringed";
    for (std::size_t index = 1; index < picture.reasons.size(); ++index)
    {
        EXPECT_TRUE(picture.IsReasonValue(picture.reasons[index].cell)) << picture.reasons[index].Text();
    }
}

TEST(Controller, AStepChosenInTheListIsLitAndOnePastTheEndMeansTheLast)
{
    Controller controller;
    EnterPuzzle(controller, kXWing);
    controller.Clue();
    controller.Clue();
    const std::size_t count = controller.game().steps().size();
    ASSERT_GE(count, 2u);
    controller.ShowStep(0);
    EXPECT_EQ(controller.shown_step(), std::optional<std::size_t>(0));
    EXPECT_EQ(controller.Picture().pattern, controller.game().steps().front().pattern);
    controller.ShowStep(count + 5);
    EXPECT_EQ(controller.shown_step(), std::optional<std::size_t>(count - 1));
    controller.ShowStep(std::nullopt);
    EXPECT_EQ(controller.shown_step(), std::optional<std::size_t>(count - 1));
}

TEST(Controller, SolvesAndSaysInHowManySteps)
{
    Controller controller;
    EnterPuzzle(controller, kXWing);
    controller.Solve();
    EXPECT_TRUE(controller.game().IsSolved());
    EXPECT_EQ(controller.Message(), "Solved in " + std::to_string(controller.game().steps().size()) + " steps.");
    EXPECT_EQ(controller.Headline(), "Solved");
    EXPECT_FALSE(controller.CanRun());
}

TEST(Controller, SaysWhyASolveStopsShort)
{
    Controller controller;
    EnterPuzzle(controller, kEscargot);
    controller.Solve();
    EXPECT_FALSE(controller.game().IsSolved());
    EXPECT_EQ(controller.Message(),
              "Stuck: the puzzle has one solution, but it needs a deeper chain than this solver follows.");

    Controller few;
    few.Enter(0, 1);
    few.Solve();
    EXPECT_EQ(few.Message(), "Stuck: this puzzle has more than one solution, so no deduction can finish it.");
}

TEST(Controller, ChangingAGivenAfterTheSolverSaysItsValuesWereCleared)
{
    Controller controller;
    EnterPuzzle(controller, kXWing);
    controller.Clue();
    const int empty =
        static_cast<int>(std::find(controller.game().givens().begin(), controller.game().givens().end(), 0) -
                         controller.game().givens().begin());
    controller.Enter(empty, 0); // an empty square cleared: nothing changes, nothing said
    EXPECT_FALSE(controller.game().steps().empty());
    controller.Enter(geometry::Cell(0, 0), 0);
    controller.Enter(geometry::Cell(0, 3), 0);
    EXPECT_TRUE(controller.game().steps().empty());
    EXPECT_FALSE(controller.shown_step().has_value());
    controller.Clue();
    controller.Enter(geometry::Cell(0, 3), 1);
    EXPECT_EQ(controller.Message(),
              "The givens changed, so the solver's values were cleared: Clue or Solve finds them again.");
}

TEST(Controller, ClashingGivensAreSaidAndStopTheSolver)
{
    Controller controller;
    controller.Enter(geometry::Cell(0, 0), 5);
    controller.Enter(geometry::Cell(0, 8), 5);
    EXPECT_EQ(controller.Headline(), "Givens clash");
    EXPECT_EQ(controller.Message(), "The givens clash: 5 is twice in row 9 (a9, i9).");
    EXPECT_FALSE(controller.CanRun());
    EXPECT_EQ(controller.Picture().clashes, (std::vector<int>{geometry::Cell(0, 0), geometry::Cell(0, 8)}));
    controller.Undo();
    EXPECT_EQ(controller.Message(), "Undone.");
    EXPECT_TRUE(controller.CanRun());
    EXPECT_TRUE(controller.Picture().clashes.empty());
}

TEST(Controller, AnImpossibleBoardIsNamed)
{
    Controller controller;
    for (int column = 0; column < 8; ++column)
    {
        controller.Enter(geometry::Cell(0, column), column + 1);
    }
    controller.Enter(geometry::Cell(1, 8), 9);
    EXPECT_EQ(controller.Headline(), "Impossible");
    EXPECT_EQ(controller.Message(), "Impossible: i9 has no digit left.");
    EXPECT_FALSE(controller.CanRun());
}

TEST(Controller, ChecksTheSolutions)
{
    Controller controller;
    controller.Check();
    EXPECT_EQ(controller.Message(), "This puzzle has more than one solution: it needs more givens.");
    EnterPuzzle(controller, kXWing);
    controller.Check();
    EXPECT_EQ(controller.Message(), "This puzzle has exactly one solution.");
    Values solution{};
    ASSERT_EQ(controller.game().CountSolutions(&solution), SolutionCount::kOne);
    controller.Enter(geometry::Cell(0, 0), solution[0] % 9 + 1); // a digit the one solution rules out
    controller.Check();
    EXPECT_EQ(controller.Message(), "This puzzle has no solution.");
}

TEST(Controller, UndoesClearsAndSaysSo)
{
    Controller controller;
    controller.Undo(); // nothing to undo: nothing said
    EXPECT_EQ(controller.Message(), "Choose a square and type a digit, or open a puzzle.");
    EnterPuzzle(controller, kXWing);
    controller.Clear();
    EXPECT_EQ(controller.Message(), "Board cleared.");
    EXPECT_EQ(controller.game().FilledCount(), 0);
    controller.Undo();
    EXPECT_EQ(controller.game().FilledCount(), 20);
}

TEST(Controller, OpensAndSavesPuzzlesAndSaysWhyAFileIsNot)
{
    const std::filesystem::path text = Scratch("xwing.sdk");
    {
        std::ofstream file(text, std::ios::binary);
        file << puzzle_file::Format(puzzle_file::Parse(kXWing));
    }
    Controller controller;
    EXPECT_TRUE(controller.Open(text));
    EXPECT_EQ(controller.Message(), "Opened sudoku-controller-xwing.sdk.");
    EXPECT_EQ(controller.file_name(), "sudoku-controller-xwing.sdk");
    EXPECT_EQ(controller.game().FilledCount(), 20);

    const std::filesystem::path board = Scratch("saved.sud");
    controller.Clue(); // the solver's values are not saved: the givens are
    EXPECT_TRUE(controller.Save(board));
    EXPECT_EQ(controller.Message(), "Saved sudoku-controller-saved.sud.");
    EXPECT_EQ(controller.file_name(), "sudoku-controller-saved.sud");
    EXPECT_EQ(puzzle_file::Read(board), controller.game().givens());
    EXPECT_EQ(std::filesystem::file_size(board), 81u);

    const std::filesystem::path bad = Scratch("bad.sdk");
    {
        std::ofstream file(bad, std::ios::binary);
        file << "123";
    }
    const int filled = controller.game().FilledCount();
    EXPECT_FALSE(controller.Open(bad));
    EXPECT_EQ(controller.Message().rfind("sudoku-controller-bad.sdk is not a Sudoku puzzle: ", 0), 0u);
    EXPECT_EQ(controller.game().FilledCount(), filled); // the board as it was
    EXPECT_FALSE(controller.Save(Scratch("no-such-folder") / "x.sud"));
    EXPECT_EQ(controller.Message().rfind("The puzzle was not saved: ", 0), 0u);

    controller.Clear();
    EXPECT_TRUE(controller.file_name().empty());
    std::filesystem::remove(text);
    std::filesystem::remove(board);
    std::filesystem::remove(bad);
}

TEST(Controller, KeepsTheChoicesForTheNextLaunch)
{
    Controller controller(Settings{false, 125});
    EXPECT_FALSE(controller.settings().shows_pencil_marks);
    EXPECT_FALSE(controller.Picture().shows_pencil_marks);
    controller.SetPencilMarks(true);
    EXPECT_TRUE(controller.Picture().shows_pencil_marks);
    controller.SetWindowPercent(0); // dragged to another size: the last one kept
    EXPECT_EQ(controller.settings().window_percent, 125);
    controller.SetWindowPercent(75);
    EXPECT_EQ(controller.settings().window_percent, 75);
}

TEST(Controller, ThePictureCarriesTheValuesTheMarksAndTheStruckCandidates)
{
    Controller controller;
    EnterPuzzle(controller, kXWing);
    const BoardPicture start = controller.Picture();
    EXPECT_TRUE(start.is_given[3]);
    EXPECT_EQ(start.values[3], 1);
    EXPECT_FALSE(digits::Contains(start.candidates[0], 1)); // d9 is 1
    // Steps until one strikes candidates, then that step lit.
    for (int clue = 0; clue < 20 && controller.Picture().struck.empty(); ++clue)
    {
        controller.Clue();
        for (std::size_t index = 0; index < controller.game().steps().size(); ++index)
        {
            controller.ShowStep(index);
            if (!controller.Picture().struck.empty())
            {
                break;
            }
        }
    }
    const BoardPicture lit = controller.Picture();
    ASSERT_FALSE(lit.struck.empty());
    const Candidate struck = lit.struck.front();
    EXPECT_FALSE(digits::Contains(lit.candidates[static_cast<std::size_t>(struck.cell)], struck.digit));
    // The marks the step struck and those it read, in the squares the rest of the clue left empty, drawn as such - with
    // the pencil marks shown or hidden.
    ASSERT_FALSE(lit.reasons.empty());
    BoardPicture hidden       = lit;
    hidden.shows_pencil_marks = false;
    int drawn                 = 0;
    for (const std::vector<Candidate> *marks : {&lit.struck, &lit.reasons})
    {
        const MarkLook look = marks == &lit.struck ? MarkLook::kStruck : MarkLook::kReason;
        for (const Candidate &mark : *marks)
        {
            if (lit.values[static_cast<std::size_t>(mark.cell)] != 0)
            {
                continue;
            }
            const bool     is_struck = std::find(lit.struck.begin(), lit.struck.end(), mark) != lit.struck.end();
            const MarkLook expected  = is_struck ? MarkLook::kStruck : look;
            EXPECT_EQ(lit.LookOf(mark.cell, mark.digit), expected) << mark.Text();
            EXPECT_EQ(hidden.LookOf(mark.cell, mark.digit), expected) << mark.Text();
            drawn++;
        }
    }
    EXPECT_GT(drawn, 0);
}

TEST(BoardPicture, DrawsEachMarkAndRingsTheValuesTheStepRead)
{
    BoardPicture picture;
    picture.candidates.fill(digits::kAll);
    picture.values[0]     = 5; // a9 holds 5
    picture.values[1]     = 7; // b9 holds 7, placed by the step
    picture.placed        = {1};
    picture.struck        = {{2, 3}};
    picture.reasons       = {{0, 5}, {1, 7}, {2, 4}, {2, 3}, {3, 6}};
    picture.candidates[3] = digits::Bit(1); // d9's 6 read, though struck since
    EXPECT_EQ(picture.LookOf(0, 5), MarkLook::kHidden) << "a square with a value shows no marks";
    EXPECT_EQ(picture.LookOf(2, 3), MarkLook::kStruck) << "struck outranks read";
    EXPECT_EQ(picture.LookOf(2, 4), MarkLook::kReason);
    EXPECT_EQ(picture.LookOf(2, 5), MarkLook::kPlain);
    EXPECT_EQ(picture.LookOf(3, 6), MarkLook::kReason);
    EXPECT_EQ(picture.LookOf(3, 2), MarkLook::kHidden) << "not a candidate";
    picture.shows_pencil_marks = false;
    EXPECT_EQ(picture.LookOf(2, 5), MarkLook::kHidden);
    EXPECT_EQ(picture.LookOf(2, 4), MarkLook::kReason);
    EXPECT_TRUE(picture.IsReasonValue(0));
    EXPECT_FALSE(picture.IsReasonValue(1)) << "placed";
    EXPECT_FALSE(picture.IsReasonValue(2)) << "empty";
    picture.values[0] = 6;
    EXPECT_FALSE(picture.IsReasonValue(0)) << "a value the step did not read";
}

TEST(Menus, TickAndEnableAsThePuzzleStands)
{
    Controller controller;
    MenuState  state = MenuStateOf(controller, WindowSizeMenu{});
    EXPECT_FALSE(state.can_undo);
    EXPECT_TRUE(state.can_run);
    reckless::app::MenuBar bar = BuildMenuBar(state);
    ASSERT_EQ(bar.menus.size(), 5u);
    EXPECT_EQ(bar.menus[0].title, "File");
    EXPECT_EQ(bar.menus[2].title, "Puzzle");
    EXPECT_EQ(bar.about_id, kAbout);
    const auto find = [&bar](int command) -> const reckless::app::MenuEntry * {
        for (const reckless::app::MenuEntry &menu : bar.menus)
        {
            for (const reckless::app::MenuEntry &entry : menu.submenu)
            {
                if (entry.id == command && !entry.separator)
                {
                    return &entry;
                }
            }
        }
        return nullptr;
    };
    ASSERT_NE(find(kUndo), nullptr);
    EXPECT_FALSE(find(kUndo)->enabled);
    EXPECT_TRUE(find(kPencilMarks)->checked);
    EXPECT_TRUE(find(kClue)->enabled);

    controller.Enter(0, 5);
    controller.Enter(8, 5);
    bar = BuildMenuBar(MenuStateOf(controller, WindowSizeMenu{}));
    EXPECT_TRUE(find(kUndo)->enabled);
    EXPECT_FALSE(find(kClue)->enabled) << "clashing givens";
    EXPECT_FALSE(find(kSolve)->enabled);
}

TEST(Menus, RunTheirCommands)
{
    Controller       controller;
    int              opens = 0, saves = 0, quits = 0, abouts = 0;
    std::vector<int> sizes;
    CommandHandlers  handlers;
    handlers.open        = [&] { ++opens; };
    handlers.save        = [&] { ++saves; };
    handlers.quit        = [&] { ++quits; };
    handlers.about       = [&] { ++abouts; };
    handlers.window_size = [&](int percent) { sizes.push_back(percent); };
    WindowSizeMenu window;
    window.larger  = 125;
    window.smaller = 75;
    const std::vector<int> commands{
        kOpen, kSave, kQuit, kAbout, kLarger, kSmaller, kWindowSizeBase + 150, kWindowSizeBase + 80};
    for (const int command : commands)
    {
        RunCommand(command, controller, handlers, window);
    }
    EXPECT_EQ(opens, 1);
    EXPECT_EQ(saves, 1);
    EXPECT_EQ(quits, 1);
    EXPECT_EQ(abouts, 1);
    EXPECT_EQ(sizes, (std::vector<int>{125, 75, 150})) << "80 % is not offered";

    EnterPuzzle(controller, kXWing);
    RunCommand(kClue, controller, handlers, window);
    EXPECT_FALSE(controller.game().steps().empty());
    RunCommand(kUndo, controller, handlers, window);
    EXPECT_TRUE(controller.game().steps().empty());
    RunCommand(kPencilMarks, controller, handlers, window);
    EXPECT_FALSE(controller.settings().shows_pencil_marks);
    RunCommand(kSolve, controller, handlers, window);
    EXPECT_TRUE(controller.game().IsSolved());
    RunCommand(kClue, controller, handlers, window); // solved: nothing to find
    RunCommand(kCheck, controller, handlers, window);
    EXPECT_EQ(controller.Message(), "This puzzle has exactly one solution.");
    RunCommand(kClearBoard, controller, handlers, window);
    EXPECT_EQ(controller.game().FilledCount(), 0);
    RunCommand(kNoCommand, controller, CommandHandlers{}, window); // nothing, and no handler is asked for
    RunCommand(kOpen, controller, CommandHandlers{}, window);
}

} // namespace
} // namespace sudoku::app
