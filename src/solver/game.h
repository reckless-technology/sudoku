#pragma once
/// @file game.h The board a person works on: the givens, what the solver has deduced from them, the steps it took, and
/// the undo history.

#include "solver/brute_force.h"
#include "solver/grid.h"
#include "solver/step.h"

#include <optional>
#include <span>
#include <string>
#include <vector>

namespace sudoku
{

/**
 * The board a person works on: the values they entered (the givens), what the solver has deduced from them, the steps
 * it took, and the undo history. Every change to the givens rebuilds the board from the givens alone, so nothing
 * deduced from a value outlives the value: the deductions are dropped, and a clue or a solve finds them again.
 */
class Game
{
  public:
    /** An empty board. */
    Game() = default;

    /** @return The givens, 0 for an empty square. */
    const Values &givens() const noexcept
    {
        return givens_;
    }

    /** @return The squares whose given clashes with another in one of its units, in order; empty for none. */
    const std::vector<int> &clashes() const noexcept
    {
        return clashes_;
    }

    /** @return The steps the solver has taken since the givens last changed, oldest first. */
    const std::vector<Step> &steps() const noexcept
    {
        return steps_;
    }

    /** @return Whether there is a change to undo. */
    bool CanUndo() const noexcept
    {
        return !history_.empty();
    }

    /** @return Whether every square has a value and the board holds no contradiction. */
    bool IsSolved() const;

    /** @return What makes the board impossible, in words (Grid::FindContradiction); none while it is not. */
    std::optional<std::string> Contradiction() const;

    /**
     * @param cell A square.
     * @return Its value, given or deduced; 0 while it is empty.
     */
    int Value(int cell) const;

    /**
     * @param cell A square.
     * @return Whether its value was entered rather than deduced.
     */
    bool IsGiven(int cell) const;

    /**
     * @param cell A square.
     * @return The digits it may still take, the pencil marks; none while the givens clash.
     */
    int Candidates(int cell) const;

    /** @return How many squares have a value, given or deduced. */
    int FilledCount() const;

    /**
     * Enters a given, or clears a square, and rebuilds the board from the givens. Clearing a square whose value was
     * deduced drops the deductions.
     * @param cell A square.
     * @param digit Its value, 1 to 9; 0 to clear it.
     * @return Whether anything changed: false for the same given again, or clearing a square already empty.
     * @throws std::out_of_range For a digit outside 0 to 9 or a square off the board.
     */
    bool SetGiven(int cell, int digit);

    /**
     * A puzzle in place of the board.
     * @param values 81 values, 0 for an empty square.
     * @throws std::invalid_argument For other than 81 values, or one outside 0 to 9.
     */
    void Load(std::span<const int> values);

    /** Every square emptied. */
    void Clear();

    /**
     * Steps until one places a value.
     * @return The steps taken; empty when there was nothing to find, or the givens clash - and then nothing to undo.
     */
    std::vector<Step> Clue();

    /**
     * Steps until the board is complete or nothing more can be found.
     * @return The steps taken; empty when there was nothing to find, or the givens clash - and then nothing to undo.
     */
    std::vector<Step> Solve();

    /**
     * How many solutions the givens have, by search (CountSolutions).
     * @param solution Where to put a solution, where there is one or more; null for nowhere.
     * @return None, one or many.
     */
    SolutionCount CountSolutions(Values *solution = nullptr) const;

    /**
     * The last change - a given, a load, a clear, a clue or a solve - taken back.
     * @return Whether there was one.
     */
    bool Undo();

  private:
    /** What a change takes back to. */
    struct Snapshot
    {
        Values            givens;
        Grid              grid;
        std::vector<Step> steps;
    };

    /** The state as it is, kept for undo. */
    void Remember();

    /** The board rebuilt from the givens, the clashes found, the steps dropped. */
    void Rebuild();

    Values                givens_{}; ///< The givens.
    Grid                  grid_;     ///< The givens and what the solver has deduced.
    std::vector<int>      clashes_;  ///< The squares whose given clashes.
    std::vector<Step>     steps_;    ///< The steps since the givens last changed.
    std::vector<Snapshot> history_;  ///< What undo takes back to, the latest last.
};

} // namespace sudoku
