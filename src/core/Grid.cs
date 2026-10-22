namespace Sudoku;

/// <summary>
/// A board being solved: each square's value, 0 while it is empty, and the digits it may still take - its candidates.
/// Placing a digit strikes it from the candidates of the 20 squares that share a unit with the square.
/// </summary>
public sealed class Grid
{
    private readonly int[] values = new int[Geometry.CellCount];
    private readonly int[] candidates = new int[Geometry.CellCount];

    /// <summary>An empty board: every square may take every digit.</summary>
    public Grid()
    {
        Array.Fill(candidates, Digits.All);
    }

    private Grid(Grid other)
    {
        other.values.CopyTo(values, 0);
        other.candidates.CopyTo(candidates, 0);
    }

    /// <summary>
    /// A board with these values placed. Values that clash are placed all the same, so the board holds the
    /// contradiction for <see cref="FindContradiction"/> to name.
    /// </summary>
    /// <param name="values">81 values, square <c>column + 9 * row</c>, 0 for an empty square.</param>
    public static Grid FromValues(IReadOnlyList<int> values)
    {
        if (values.Count != Geometry.CellCount)
        {
            throw new ArgumentException($"a board has {Geometry.CellCount} squares, not {values.Count}", nameof(values));
        }
        var grid = new Grid();
        for (int cell = 0; cell < Geometry.CellCount; cell++)
        {
            if (values[cell] != 0)
            {
                grid.Place(cell, values[cell]);
            }
        }
        return grid;
    }

    /// <returns>A copy, to change without changing this one.</returns>
    public Grid Clone() => new(this);

    /// <param name="cell">A square.</param>
    /// <returns>Its value, 1 to 9, or 0 while it is empty.</returns>
    public int Value(int cell) => values[cell];

    /// <param name="cell">A square.</param>
    /// <returns>The digits it may take: its value's alone once it has one.</returns>
    public int Candidates(int cell) => candidates[cell];

    /// <param name="cell">A square.</param>
    /// <returns>Whether it has no value yet.</returns>
    public bool IsEmpty(int cell) => values[cell] == 0;

    /// <summary>Whether every square has a value.</summary>
    public bool IsComplete => Array.IndexOf(values, 0) < 0;

    /// <summary>The values, square <c>column + 9 * row</c>, 0 for an empty square.</summary>
    public int[] Values() => (int[])values.Clone();

    /// <summary>Gives a square its value, and strikes the digit from the candidates of the squares around it.</summary>
    /// <param name="cell">A square.</param>
    /// <param name="digit">Its value, 1 to 9.</param>
    public void Place(int cell, int digit)
    {
        values[cell] = digit;
        candidates[cell] = Digits.Bit(digit);
        foreach (int peer in Geometry.Peers(cell))
        {
            if (values[peer] == 0)
            {
                candidates[peer] &= ~Digits.Bit(digit);
            }
        }
    }

    /// <summary>Strikes a candidate from an empty square.</summary>
    /// <param name="cell">A square.</param>
    /// <param name="digit">A digit.</param>
    /// <returns>Whether it was a candidate there, and is gone.</returns>
    public bool Eliminate(int cell, int digit)
    {
        if (values[cell] != 0 || !Digits.Contains(candidates[cell], digit))
        {
            return false;
        }
        candidates[cell] &= ~Digits.Bit(digit);
        return true;
    }

    /// <param name="unit">A unit.</param>
    /// <param name="digit">A digit.</param>
    /// <returns>Whether a square of the unit has the digit as its value.</returns>
    public bool HasPlaced(Unit unit, int digit) => unit.Cells.Any(cell => values[cell] == digit);

    /// <param name="unit">A unit.</param>
    /// <param name="digit">A digit.</param>
    /// <returns>The unit's empty squares that may still take the digit.</returns>
    public List<int> PlacesFor(Unit unit, int digit) =>
        unit.Cells.Where(cell => values[cell] == 0 && Digits.Contains(candidates[cell], digit)).ToList();

    /// <summary>
    /// The first thing that makes the board impossible: a digit twice in a unit, an empty square with no candidate, or
    /// a digit with no place left in a unit.
    /// </summary>
    /// <returns>What it is, in words; null for none.</returns>
    public string? FindContradiction()
    {
        foreach (Unit unit in Geometry.Units)
        {
            foreach (IGrouping<int, int> twice in unit.Cells.Where(cell => values[cell] != 0)
                         .GroupBy(cell => values[cell])
                         .Where(group => group.Count() > 1))
            {
                return $"{twice.Key} is twice in {unit} ({Geometry.Names(twice)})";
            }
        }
        for (int cell = 0; cell < Geometry.CellCount; cell++)
        {
            if (values[cell] == 0 && candidates[cell] == 0)
            {
                return $"{Geometry.Name(cell)} has no digit left";
            }
        }
        foreach (Unit unit in Geometry.Units)
        {
            for (int digit = 1; digit <= 9; digit++)
            {
                if (!HasPlaced(unit, digit) && PlacesFor(unit, digit).Count == 0)
                {
                    return $"{digit} has no place left in {unit}";
                }
            }
        }
        return null;
    }

    /// <param name="values">81 values, 0 for an empty square.</param>
    /// <returns>The squares whose value another square in one of its units also has, in order.</returns>
    public static IReadOnlyList<int> Clashes(IReadOnlyList<int> values) =>
        Enumerable.Range(0, Geometry.CellCount)
            .Where(cell => values[cell] != 0 && Geometry.Peers(cell).Any(peer => values[peer] == values[cell]))
            .ToArray();
}
