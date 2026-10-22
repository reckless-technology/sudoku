using Sudoku.Techniques;

namespace Sudoku;

/// <summary>
/// Solves a board by deduction, a step at a time, as a person does: each step is the first deduction found by the
/// simplest technique that finds one, and after it the search starts again from the simplest. It never guesses.
/// </summary>
public sealed class Solver
{
    /// <summary>A technique: the first deduction it finds on the board, without changing it; null for none.</summary>
    public delegate Step? Finder(Grid grid);

    /// <summary>The techniques, in the order they are tried: the simplest first.</summary>
    public static readonly IReadOnlyList<Finder> Finders =
    [
        Singles.Naked,
        Singles.Hidden,
        Subsets.Pairs,
        Intersections.Find,
        Subsets.Triples,
        Subsets.Quads,
        Fish.XWing,
        Fish.Swordfish,
        Fish.Jellyfish,
        Wings.XY,
        Wings.XYZ,
        Chains.SimpleColouring,
        Chains.XYChain,
        Forcing.Nishio,
        Forcing.ForcingChain,
        Forcing.ContradictionChain,
    ];

    /// <param name="grid">The board to solve; the solver changes it.</param>
    public Solver(Grid grid)
    {
        Grid = grid;
    }

    /// <summary>The board, as far as it is solved.</summary>
    public Grid Grid { get; }

    /// <summary>What makes the board impossible, in words; null while it is not.</summary>
    public string? Contradiction => Grid.FindContradiction();

    /// <summary>The next deduction, made on the board.</summary>
    /// <returns>The step; null when the board is complete, contradicts itself, or no technique finds anything.</returns>
    public Step? NextStep()
    {
        if (Grid.IsComplete || Contradiction is not null)
        {
            return null;
        }
        foreach (Finder finder in Finders)
        {
            if (finder(Grid) is { } step)
            {
                Apply(Grid, step);
                return step;
            }
        }
        return null;
    }

    /// <summary>Steps until one places a value: a clue.</summary>
    /// <returns>The steps taken, the placement last; empty when there was nothing to find.</returns>
    public IReadOnlyList<Step> Clue()
    {
        var steps = new List<Step>();
        while (NextStep() is { } step)
        {
            steps.Add(step);
            if (step.IsPlacement)
            {
                break;
            }
        }
        return steps;
    }

    /// <summary>Steps until the board is complete or nothing more can be found.</summary>
    /// <returns>The steps taken.</returns>
    public IReadOnlyList<Step> Solve()
    {
        var steps = new List<Step>();
        while (NextStep() is { } step)
        {
            steps.Add(step);
        }
        return steps;
    }

    /// <summary>A step's placements and eliminations made on a board.</summary>
    public static void Apply(Grid grid, Step step)
    {
        foreach (Candidate placement in step.Placements)
        {
            grid.Place(placement.Cell, placement.Digit);
        }
        foreach (Candidate elimination in step.Eliminations)
        {
            grid.Eliminate(elimination.Cell, elimination.Digit);
        }
    }
}
