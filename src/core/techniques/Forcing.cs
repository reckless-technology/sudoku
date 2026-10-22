namespace Sudoku.Techniques;

/// <summary>
/// Trying a candidate and following where it leads. Nishio follows one digit alone; the chains follow every single the
/// trial gives. These come last: a person reaches for them when the patterns run out.
/// </summary>
public static class Forcing
{
    /// <summary>
    /// Nishio: a digit placed in a square, and the digit's own hidden singles followed, leaves a unit with no place
    /// for it - so the square is not that digit.
    /// </summary>
    public static Step? Nishio(Grid grid)
    {
        for (int digit = 1; digit <= 9; digit++)
        {
            for (int cell = 0; cell < Geometry.CellCount; cell++)
            {
                if (!grid.IsEmpty(cell) || !Digits.Contains(grid.Candidates(cell), digit))
                {
                    continue;
                }
                Grid trial = grid.Clone();
                trial.Place(cell, digit);
                if (FollowDigit(trial, digit) is { } reason)
                {
                    string name = Geometry.Name(cell);
                    return new Step(Technique.Nishio,
                                    $"Nishio: were {name} {digit}, {digit}'s own singles leave {reason}, so {name} is not " +
                                        $"{digit}",
                                    [], [new Candidate(cell, digit)], [cell]);
                }
            }
        }
        return null;
    }

    /// <summary>
    /// Forcing chain: a square with two or three candidates, each tried in turn and its singles followed. When every
    /// trial places the same value in another square, or strikes the same candidate, so does the board.
    /// </summary>
    public static Step? ForcingChain(Grid grid)
    {
        foreach (int cell in EmptyByCandidateCount(grid).Where(cell => Digits.Count(grid.Candidates(cell)) <= 3))
        {
            int[] digits = Digits.Each(grid.Candidates(cell)).ToArray();
            var outcomes = new List<Grid>();
            foreach (int digit in digits)
            {
                Grid trial = grid.Clone();
                trial.Place(cell, digit);
                if (Propagate(trial) is null)
                {
                    outcomes.Add(trial);
                }
            }
            if (outcomes.Count != digits.Length)
            {
                continue; // a trial that fails is the contradiction chain's to report
            }
            string name = Geometry.Name(cell);
            string choices = Digits.Format(grid.Candidates(cell));
            for (int other = 0; other < Geometry.CellCount; other++)
            {
                int value = outcomes[0].Value(other);
                if (grid.IsEmpty(other) && value != 0 && outcomes.All(outcome => outcome.Value(other) == value))
                {
                    return new Step(Technique.ForcingChain,
                                    $"Forcing chain: whichever of {choices} {name} takes, the singles that follow give " +
                                        $"{Geometry.Name(other)} = {value}",
                                    [new Candidate(other, value)], [], [cell, other]);
                }
            }
            var eliminations = new List<Candidate>();
            for (int other = 0; other < Geometry.CellCount; other++)
            {
                if (!grid.IsEmpty(other))
                {
                    continue;
                }
                foreach (int digit in Digits.Each(grid.Candidates(other)))
                {
                    if (outcomes.All(outcome => !Digits.Contains(outcome.Candidates(other), digit)))
                    {
                        eliminations.Add(new Candidate(other, digit));
                    }
                }
            }
            if (eliminations.Count > 0)
            {
                return new Step(Technique.ForcingChain,
                                $"Forcing chain: whichever of {choices} {name} takes, the singles that follow " +
                                    TechniqueNames.Removes(eliminations).Replace("removes", "remove"),
                                [], eliminations, [cell]);
            }
        }
        return null;
    }

    /// <summary>
    /// Contradiction chain: a candidate tried, and the singles that follow lead to a contradiction - so the square is
    /// not that digit. The last resort.
    /// </summary>
    public static Step? ContradictionChain(Grid grid)
    {
        foreach (int cell in EmptyByCandidateCount(grid))
        {
            foreach (int digit in Digits.Each(grid.Candidates(cell)))
            {
                Grid trial = grid.Clone();
                trial.Place(cell, digit);
                if (Propagate(trial) is { } reason)
                {
                    string name = Geometry.Name(cell);
                    return new Step(Technique.ContradictionChain,
                                    $"Contradiction chain: were {name} {digit}, the singles that follow leave {reason}, so " +
                                        $"{name} is not {digit}",
                                    [], [new Candidate(cell, digit)], [cell]);
                }
            }
        }
        return null;
    }

    /// <summary>Naked and hidden singles placed until none is left, or the board contradicts itself.</summary>
    /// <returns>The contradiction met, in words; null for none.</returns>
    internal static string? Propagate(Grid grid)
    {
        while (true)
        {
            if (grid.FindContradiction() is { } reason)
            {
                return reason;
            }
            if ((Singles.Naked(grid) ?? Singles.Hidden(grid)) is not { } single)
            {
                return null;
            }
            foreach (Candidate placement in single.Placements)
            {
                grid.Place(placement.Cell, placement.Digit);
            }
        }
    }

    /// <summary>One digit's hidden singles placed until none is left, or a unit has no place for it.</summary>
    /// <returns>The unit left with no place, in words; null for none.</returns>
    private static string? FollowDigit(Grid grid, int digit)
    {
        bool hasPlaced = true;
        while (hasPlaced)
        {
            hasPlaced = false;
            foreach (Unit unit in Geometry.Units)
            {
                if (grid.HasPlaced(unit, digit))
                {
                    continue;
                }
                List<int> places = grid.PlacesFor(unit, digit);
                if (places.Count == 0)
                {
                    return $"no place for {digit} in {unit}";
                }
                if (places.Count == 1)
                {
                    grid.Place(places[0], digit);
                    hasPlaced = true;
                }
            }
        }
        return null;
    }

    /// <returns>The empty squares, those with the fewest candidates first.</returns>
    private static IEnumerable<int> EmptyByCandidateCount(Grid grid) =>
        Enumerable.Range(0, Geometry.CellCount)
            .Where(grid.IsEmpty)
            .OrderBy(cell => Digits.Count(grid.Candidates(cell)))
            .ThenBy(cell => cell);
}
