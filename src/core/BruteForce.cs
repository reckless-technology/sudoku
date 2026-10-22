namespace Sudoku;

/// <summary>How many solutions a puzzle has, as far as it matters: none, one, or more than one.</summary>
public enum SolutionCount
{
    /// <summary>No way to complete the board.</summary>
    None,

    /// <summary>Exactly one: a proper puzzle.</summary>
    One,

    /// <summary>More than one: the givens are too few to decide it.</summary>
    Many,
}

/// <summary>
/// Search by trial: not how the solver solves, but how it checks a puzzle - whether it has a solution, and only one,
/// and what that solution is.
/// </summary>
public static class BruteForce
{
    /// <param name="values">81 values, 0 for an empty square.</param>
    /// <param name="solution">The first solution found; null for none.</param>
    /// <returns>Whether the puzzle has no solution, one, or more than one.</returns>
    public static SolutionCount Count(IReadOnlyList<int> values, out int[]? solution)
    {
        solution = null;
        Grid grid = Grid.FromValues(values);
        if (grid.FindContradiction() is not null)
        {
            return SolutionCount.None;
        }
        int found = Search(grid, 2, ref solution);
        return found switch
        {
            0 => SolutionCount.None,
            1 => SolutionCount.One,
            _ => SolutionCount.Many,
        };
    }

    /// <summary>Depth first, always at the empty square with the fewest candidates.</summary>
    /// <returns>The solutions found, up to <paramref name="limit"/>.</returns>
    private static int Search(Grid grid, int limit, ref int[]? first)
    {
        int best = -1;
        int fewest = 10;
        for (int cell = 0; cell < Geometry.CellCount; cell++)
        {
            if (!grid.IsEmpty(cell))
            {
                continue;
            }
            int count = Digits.Count(grid.Candidates(cell));
            if (count == 0)
            {
                return 0;
            }
            if (count < fewest)
            {
                fewest = count;
                best = cell;
            }
        }
        if (best < 0)
        {
            first ??= grid.Values();
            return 1;
        }
        int total = 0;
        foreach (int digit in Digits.Each(grid.Candidates(best)))
        {
            Grid trial = grid.Clone();
            trial.Place(best, digit);
            total += Search(trial, limit - total, ref first);
            if (total >= limit)
            {
                break;
            }
        }
        return total;
    }
}
