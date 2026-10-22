#include "app/controller.h"

#include "app/paths.h"
#include "solver/puzzle_file.h"

#include <algorithm>
#include <utility>

namespace sudoku::app
{

Controller::Controller(Settings settings) : settings_(std::move(settings))
{
}

void Controller::Select(int cell)
{
    selected_ = std::clamp(cell, 0, geometry::kCellCount - 1);
}

void Controller::MoveSelection(int rows, int columns)
{
    const int row    = std::clamp(geometry::Row(selected_) + rows, 0, 8);
    const int column = std::clamp(geometry::Column(selected_) + columns, 0, 8);
    selected_        = geometry::Cell(row, column);
}

void Controller::Enter(int cell, int digit)
{
    const bool had_deductions = !game_.steps().empty();
    if (!game_.SetGiven(cell, digit))
    {
        return;
    }
    Said(had_deductions ? std::optional<std::string>("The givens changed, so the solver's values were cleared: Clue "
                                                     "or Solve finds them again.")
                        : std::nullopt);
}

void Controller::Clue()
{
    const std::vector<Step> steps = game_.Clue();
    Said(!steps.empty() ? steps.back().description : WhyStuck());
}

void Controller::Solve()
{
    const std::vector<Step> steps = game_.Solve();
    Said(game_.IsSolved() ? "Solved in " + std::to_string(steps.size()) + " steps." : WhyStuck());
}

void Controller::Check()
{
    switch (game_.CountSolutions())
    {
    case SolutionCount::kNone:
        Said("This puzzle has no solution.");
        break;
    case SolutionCount::kOne:
        Said("This puzzle has exactly one solution.");
        break;
    case SolutionCount::kMany:
        Said("This puzzle has more than one solution: it needs more givens.");
        break;
    }
}

void Controller::Undo()
{
    if (game_.Undo())
    {
        Said("Undone.");
    }
}

void Controller::Clear()
{
    game_.Clear();
    file_name_.clear();
    Said("Board cleared.");
}

bool Controller::Open(const std::filesystem::path &path)
{
    const std::string name = Utf8FromPath(path.filename());
    try
    {
        game_.Load(puzzle_file::Read(path));
    }
    catch (const puzzle_file::Error &error)
    {
        message_ = name + " is not a Sudoku puzzle: " + error.what() + ".";
        return false;
    }
    file_name_ = name;
    Said("Opened " + name + ".");
    return true;
}

bool Controller::Save(const std::filesystem::path &path)
{
    try
    {
        puzzle_file::Write(path, game_.givens());
    }
    catch (const puzzle_file::Error &error)
    {
        message_ = std::string("The puzzle was not saved: ") + error.what();
        return false;
    }
    file_name_ = Utf8FromPath(path.filename());
    message_   = "Saved " + file_name_ + ".";
    return true;
}

void Controller::ShowStep(std::optional<std::size_t> index)
{
    if (index && *index >= game_.steps().size())
    {
        index.reset();
    }
    shown_step_ =
        index ? index : (game_.steps().empty() ? std::nullopt : std::optional<std::size_t>(game_.steps().size() - 1));
}

void Controller::SetPencilMarks(bool shows)
{
    settings_.shows_pencil_marks = shows;
}

void Controller::SetWindowPercent(int percent)
{
    if (percent != 0)
    {
        settings_.window_percent = percent;
    }
}

bool Controller::CanRun() const
{
    return game_.clashes().empty() && !game_.Contradiction() && !game_.IsSolved();
}

std::string Controller::Headline() const
{
    if (!game_.clashes().empty())
    {
        return "Givens clash";
    }
    if (game_.Contradiction())
    {
        return "Impossible";
    }
    if (game_.IsSolved())
    {
        return "Solved";
    }
    const int filled = game_.FilledCount();
    return filled == 0 ? "Enter a puzzle" : std::to_string(filled) + " of 81";
}

std::string Controller::Message() const
{
    return message_ ? *message_ : Standing();
}

BoardPicture Controller::Picture() const
{
    BoardPicture picture;
    for (int cell = 0; cell < geometry::kCellCount; ++cell)
    {
        const auto index          = static_cast<std::size_t>(cell);
        picture.values[index]     = game_.Value(cell);
        picture.is_given[index]   = game_.IsGiven(cell);
        picture.candidates[index] = game_.Candidates(cell);
    }
    picture.selected           = selected_;
    picture.shows_pencil_marks = settings_.shows_pencil_marks;
    picture.clashes            = game_.clashes();
    if (shown_step_ && *shown_step_ < game_.steps().size())
    {
        const Step &step = game_.steps()[*shown_step_];
        picture.pattern  = step.pattern;
        for (const Candidate &placement : step.placements)
        {
            picture.placed.push_back(placement.cell);
        }
        picture.struck  = step.eliminations;
        picture.reasons = step.reasons;
    }
    return picture;
}

std::string Controller::WhyStuck() const
{
    if (const std::optional<std::string> contradiction = game_.Contradiction())
    {
        return "Impossible: " + *contradiction + ".";
    }
    if (game_.IsSolved())
    {
        return "Solved.";
    }
    switch (game_.CountSolutions())
    {
    case SolutionCount::kNone:
        return "Stuck: this puzzle has no solution.";
    case SolutionCount::kMany:
        return "Stuck: this puzzle has more than one solution, so no deduction can finish it.";
    case SolutionCount::kOne:
        break;
    }
    return "Stuck: the puzzle has one solution, but it needs a deeper chain than this solver follows.";
}

std::string Controller::Standing() const
{
    if (!game_.clashes().empty())
    {
        return "The givens clash: " + game_.Contradiction().value_or("") + ".";
    }
    if (const std::optional<std::string> contradiction = game_.Contradiction())
    {
        return "Impossible: " + *contradiction + ".";
    }
    if (game_.IsSolved())
    {
        return "Solved.";
    }
    const int filled = game_.FilledCount();
    return filled == 0 ? "Choose a square and type a digit, or open a puzzle."
                       : std::to_string(filled) + " of 81 squares filled.";
}

void Controller::Said(std::optional<std::string> message)
{
    message_ = std::move(message);
    ShowStep(std::nullopt);
}

} // namespace sudoku::app
