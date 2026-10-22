#include "solver/game.h"

#include "solver/solver.h"

#include <algorithm>
#include <stdexcept>

namespace sudoku
{

bool Game::IsSolved() const
{
    return grid_.IsComplete() && !grid_.FindContradiction();
}

std::optional<std::string> Game::Contradiction() const
{
    return grid_.FindContradiction();
}

int Game::Value(int cell) const
{
    int given = givens_[static_cast<std::size_t>(cell)];
    return given != 0 ? given : grid_.Value(cell);
}

bool Game::IsGiven(int cell) const
{
    return givens_[static_cast<std::size_t>(cell)] != 0;
}

int Game::Candidates(int cell) const
{
    return clashes_.empty() ? grid_.Candidates(cell) : 0;
}

int Game::FilledCount() const
{
    int count = 0;
    for (int cell = 0; cell < geometry::kCellCount; cell++)
    {
        if (Value(cell) != 0)
        {
            count++;
        }
    }
    return count;
}

bool Game::SetGiven(int cell, int digit)
{
    if (digit < 0 || digit > 9)
    {
        throw std::out_of_range("a digit is 1 to 9, or 0 to clear, not " + std::to_string(digit));
    }
    if (cell < 0 || cell >= geometry::kCellCount)
    {
        throw std::out_of_range("a square is 0 to " + std::to_string(geometry::kCellCount - 1) + ", not " +
                                std::to_string(cell));
    }
    if (givens_[static_cast<std::size_t>(cell)] == digit && (digit != 0 || grid_.IsEmpty(cell)))
    {
        return false;
    }
    Remember();
    givens_[static_cast<std::size_t>(cell)] = digit;
    Rebuild();
    return true;
}

void Game::Load(std::span<const int> values)
{
    if (values.size() != givens_.size() ||
        std::any_of(values.begin(), values.end(), [](int value) { return value < 0 || value > 9; }))
    {
        throw std::invalid_argument("a puzzle is 81 values of 0 to 9");
    }
    Remember();
    std::copy(values.begin(), values.end(), givens_.begin());
    Rebuild();
}

void Game::Clear()
{
    Load(Values{});
}

std::vector<Step> Game::Clue()
{
    if (!clashes_.empty())
    {
        return {};
    }
    Remember();
    std::vector<Step> taken = Solver(grid_).Clue();
    if (taken.empty())
    {
        history_.pop_back(); // the board is as it was: nothing to undo
    }
    steps_.insert(steps_.end(), taken.begin(), taken.end());
    return taken;
}

std::vector<Step> Game::Solve()
{
    if (!clashes_.empty())
    {
        return {};
    }
    Remember();
    std::vector<Step> taken = Solver(grid_).Solve();
    if (taken.empty())
    {
        history_.pop_back(); // the board is as it was: nothing to undo
    }
    steps_.insert(steps_.end(), taken.begin(), taken.end());
    return taken;
}

SolutionCount Game::CountSolutions(Values *solution) const
{
    return sudoku::CountSolutions(givens_, solution);
}

bool Game::Undo()
{
    if (history_.empty())
    {
        return false;
    }
    Snapshot snapshot = std::move(history_.back());
    history_.pop_back();
    givens_  = snapshot.givens;
    grid_    = snapshot.grid;
    steps_   = std::move(snapshot.steps);
    clashes_ = Grid::Clashes(givens_);
    return true;
}

void Game::Remember()
{
    history_.push_back(Snapshot{givens_, grid_, steps_});
}

void Game::Rebuild()
{
    clashes_ = Grid::Clashes(givens_);
    grid_    = Grid::FromValues(givens_);
    steps_.clear();
}

} // namespace sudoku
