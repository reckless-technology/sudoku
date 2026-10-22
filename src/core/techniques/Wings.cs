namespace Sudoku.Techniques;

/// <summary>
/// XY-wing: a pivot square with candidates {x,y} sees two pincers, {x,z} and {y,z}. Whichever of x and y the pivot takes,
/// one pincer is z, so a square that sees both pincers cannot be z. XYZ-wing: the pivot is {x,y,z} itself, so a square
/// must see the pivot as well as both pincers to lose z.
/// </summary>
public static class Wings
{
    /// <summary>The first XY-wing that strikes a candidate.</summary>
    public static Step? XY(Grid grid)
    {
        for (int pivot = 0; pivot < Geometry.CellCount; pivot++)
        {
            int pivotSet = grid.Candidates(pivot);
            if (!grid.IsEmpty(pivot) || Digits.Count(pivotSet) != 2)
            {
                continue;
            }
            int[] pincers = BivaluePeers(grid, pivot).ToArray();
            foreach (int first in pincers)
            {
                int firstSet = grid.Candidates(first);
                int shared = firstSet & pivotSet;
                int z = firstSet & ~pivotSet;
                if (Digits.Count(shared) != 1 || Digits.Count(z) != 1)
                {
                    continue;
                }
                int wanted = pivotSet & ~shared | z;
                foreach (int second in pincers.Where(cell => cell != first && grid.Candidates(cell) == wanted))
                {
                    int digit = Digits.First(z);
                    var eliminations = Seeing(grid, digit, [first, second], [pivot]);
                    if (eliminations.Count > 0)
                    {
                        return new Step(Technique.XYWing,
                                        $"XY-wing: pivot {Geometry.Name(pivot)} {Digits.Format(pivotSet)} with pincers " +
                                            $"{Geometry.Name(first)} {Digits.Format(firstSet)} and {Geometry.Name(second)} " +
                                            $"{Digits.Format(wanted)}: one pincer is {digit}, so what sees both pincers " +
                                            $"loses it - {TechniqueNames.Removes(eliminations)}",
                                        [], eliminations, [pivot, first, second]);
                    }
                }
            }
        }
        return null;
    }

    /// <summary>The first XYZ-wing that strikes a candidate.</summary>
    public static Step? XYZ(Grid grid)
    {
        for (int pivot = 0; pivot < Geometry.CellCount; pivot++)
        {
            int pivotSet = grid.Candidates(pivot);
            if (!grid.IsEmpty(pivot) || Digits.Count(pivotSet) != 3)
            {
                continue;
            }
            int[] pincers = BivaluePeers(grid, pivot).Where(cell => (grid.Candidates(cell) & ~pivotSet) == 0).ToArray();
            foreach (int[] pair in Combinations.Of(pincers, 2))
            {
                int firstSet = grid.Candidates(pair[0]);
                int secondSet = grid.Candidates(pair[1]);
                int z = firstSet & secondSet;
                if (firstSet == secondSet || Digits.Count(z) != 1 || (firstSet | secondSet) != pivotSet)
                {
                    continue;
                }
                int digit = Digits.First(z);
                var eliminations = Seeing(grid, digit, [pivot, pair[0], pair[1]], []);
                if (eliminations.Count > 0)
                {
                    return new Step(Technique.XYZWing,
                                    $"XYZ-wing: pivot {Geometry.Name(pivot)} {Digits.Format(pivotSet)} with pincers " +
                                        $"{Geometry.Name(pair[0])} {Digits.Format(firstSet)} and {Geometry.Name(pair[1])} " +
                                        $"{Digits.Format(secondSet)}: one of the three is {digit}, so what sees all three " +
                                        $"loses it - {TechniqueNames.Removes(eliminations)}",
                                    [], eliminations, [pivot, pair[0], pair[1]]);
                }
            }
        }
        return null;
    }

    /// <returns>The empty squares with two candidates that share a unit with the square.</returns>
    private static IEnumerable<int> BivaluePeers(Grid grid, int cell) =>
        Geometry.Peers(cell).Where(peer => grid.IsEmpty(peer) && Digits.Count(grid.Candidates(peer)) == 2);

    /// <returns>The digit as a candidate in every empty square that sees all of <paramref name="seen"/>, but for
    /// <paramref name="also"/>.</returns>
    internal static List<Candidate> Seeing(Grid grid, int digit, IReadOnlyList<int> seen, IReadOnlyList<int> also)
    {
        var eliminations = new List<Candidate>();
        for (int cell = 0; cell < Geometry.CellCount; cell++)
        {
            if (grid.IsEmpty(cell) && Digits.Contains(grid.Candidates(cell), digit) && !seen.Contains(cell) &&
                !also.Contains(cell) && seen.All(other => Geometry.Sees(cell, other)))
            {
                eliminations.Add(new Candidate(cell, digit));
            }
        }
        return eliminations;
    }
}
