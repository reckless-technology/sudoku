#pragma once
/// @file solver.h The solver: a board solved by deduction, a step at a time, as a person solves it.

#include "solver/grid.h"
#include "solver/step.h"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace sudoku
{

/**
 * Solves a board by deduction, a step at a time: each step is the first deduction found by the simplest technique that
 * finds one, and after it the search starts again from the simplest. It never guesses.
 */
class Solver
{
  public:
    /** A technique: the first deduction it finds on a board, which it does not change; none for none. */
    using Finder = std::function<std::optional<Step>(const Grid &)>;

    /**
     * @return The techniques in the order they are tried: singles (naked, hidden), pairs, intersections, triples,
     * quads, X-wing, swordfish, jellyfish, XY-wing, XYZ-wing, simple colouring, XY-chain, Nishio, forcing chain,
     * contradiction chain.
     */
    static const std::vector<Finder> &Finders();

    /** @param grid The board to solve, which the solver changes; outlives the solver. */
    explicit Solver(Grid &grid) : grid_(grid)
    {
    }

    /** @return The board, as far as it is solved. */
    const Grid &grid() const noexcept
    {
        return grid_;
    }

    /** @return What makes the board impossible, in words; none while it is not. */
    std::optional<std::string> Contradiction() const
    {
        return grid_.FindContradiction();
    }

    /**
     * The next deduction, made on the board.
     * @return The step; none when the board is complete, contradicts itself, or no technique finds anything.
     */
    std::optional<Step> NextStep();

    /**
     * Steps until one places a value: a clue.
     * @return The steps taken, the placement last; empty when there was nothing to find.
     */
    std::vector<Step> Clue();

    /**
     * Steps until the board is complete or nothing more can be found.
     * @return The steps taken.
     */
    std::vector<Step> Solve();

    /**
     * A step's placements and eliminations made on a board.
     * @param grid The board.
     * @param step The step.
     */
    static void Apply(Grid &grid, const Step &step);

  private:
    Grid &grid_; ///< The board.
};

} // namespace sudoku
