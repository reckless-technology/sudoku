namespace Sudoku.Techniques;

/// <summary>A square with one candidate left, and a digit with one place left in a unit.</summary>
public static class Singles
{
    /// <summary>The first empty square with one candidate left.</summary>
    public static Step? Naked(Grid grid)
    {
        for (int cell = 0; cell < Geometry.CellCount; cell++)
        {
            if (grid.IsEmpty(cell) && Digits.Count(grid.Candidates(cell)) == 1)
            {
                int digit = Digits.First(grid.Candidates(cell));
                return new Step(Technique.NakedSingle, $"Naked single: {Geometry.Name(cell)} can only be {digit}",
                                [new Candidate(cell, digit)], [], [cell]);
            }
        }
        return null;
    }

    /// <summary>The first digit with one place left in a unit, the rows first, then the columns, then the boxes.</summary>
    public static Step? Hidden(Grid grid)
    {
        foreach (Unit unit in Geometry.Units)
        {
            for (int digit = 1; digit <= 9; digit++)
            {
                if (grid.HasPlaced(unit, digit))
                {
                    continue;
                }
                List<int> places = grid.PlacesFor(unit, digit);
                if (places.Count == 1)
                {
                    return new Step(Technique.HiddenSingle,
                                    $"Hidden single: {digit} has one place in {unit}, {Geometry.Name(places[0])}",
                                    [new Candidate(places[0], digit)], [], unit.Cells);
                }
            }
        }
        return null;
    }
}
