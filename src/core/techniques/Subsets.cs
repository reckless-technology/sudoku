namespace Sudoku.Techniques;

/// <summary>
/// Naked and hidden subsets. A naked subset is n empty squares of a unit whose candidates are n digits between them:
/// those digits go in those squares, so the unit's other squares lose them. A hidden subset is n digits whose places in
/// a unit are the same n squares: those squares hold those digits, so they lose every other candidate. Each square of a
/// naked subset may hold any of its digits, not all: {1,2}, {2,3}, {1,3} is a naked triple.
/// </summary>
public static class Subsets
{
    /// <summary>The first naked pair, then the first hidden pair.</summary>
    public static Step? Pairs(Grid grid) => Naked(grid, 2) ?? Hidden(grid, 2);

    /// <summary>The first naked triple, then the first hidden triple.</summary>
    public static Step? Triples(Grid grid) => Naked(grid, 3) ?? Hidden(grid, 3);

    /// <summary>The first naked quad, then the first hidden quad.</summary>
    public static Step? Quads(Grid grid) => Naked(grid, 4) ?? Hidden(grid, 4);

    private static readonly string[] sizeNames = ["", "single", "pair", "triple", "quad"];

    /// <summary>The first naked subset of <paramref name="size"/> squares that strikes a candidate.</summary>
    public static Step? Naked(Grid grid, int size)
    {
        foreach (Unit unit in Geometry.Units)
        {
            int[] empty = unit.Cells.Where(grid.IsEmpty).ToArray();
            if (empty.Length <= size)
            {
                continue; // the unit's every empty square: nothing outside it to strike from
            }
            int[] small = empty.Where(cell => Digits.Count(grid.Candidates(cell)) <= size).ToArray();
            foreach (int[] subset in Combinations.Of(small, size))
            {
                int digits = subset.Aggregate(0, (set, cell) => set | grid.Candidates(cell));
                if (Digits.Count(digits) != size)
                {
                    continue;
                }
                var eliminations = new List<Candidate>();
                foreach (int cell in empty.Except(subset))
                {
                    eliminations.AddRange(Digits.Each(grid.Candidates(cell) & digits).Select(d => new Candidate(cell, d)));
                }
                if (eliminations.Count > 0)
                {
                    return new Step(NakedTechnique(size),
                                    $"Naked {sizeNames[size]} {Digits.Format(digits)} in {unit} ({Geometry.Names(subset)}) " +
                                        TechniqueNames.Removes(eliminations),
                                    [], eliminations, subset);
                }
            }
        }
        return null;
    }

    /// <summary>The first hidden subset of <paramref name="size"/> digits that strikes a candidate.</summary>
    public static Step? Hidden(Grid grid, int size)
    {
        foreach (Unit unit in Geometry.Units)
        {
            int[] open = Enumerable.Range(1, 9).Where(digit => !grid.HasPlaced(unit, digit)).ToArray();
            if (open.Length <= size)
            {
                continue; // every digit the unit lacks: nothing else to strike
            }
            var places = open.ToDictionary(digit => digit, digit => grid.PlacesFor(unit, digit));
            int[] few = open.Where(digit => places[digit].Count >= 1 && places[digit].Count <= size).ToArray();
            foreach (int[] subset in Combinations.Of(few, size))
            {
                int[] cells = subset.SelectMany(digit => places[digit]).Distinct().Order().ToArray();
                if (cells.Length != size)
                {
                    continue;
                }
                int digits = subset.Aggregate(0, (set, digit) => set | Digits.Bit(digit));
                var eliminations = new List<Candidate>();
                foreach (int cell in cells)
                {
                    eliminations.AddRange(Digits.Each(grid.Candidates(cell) & ~digits).Select(d => new Candidate(cell, d)));
                }
                if (eliminations.Count > 0)
                {
                    return new Step(HiddenTechnique(size),
                                    $"Hidden {sizeNames[size]} {Digits.Format(digits)} in {unit} ({Geometry.Names(cells)}) " +
                                        TechniqueNames.Removes(eliminations),
                                    [], eliminations, cells);
                }
            }
        }
        return null;
    }

    private static Technique NakedTechnique(int size) => size switch
    {
        2 => Technique.NakedPair,
        3 => Technique.NakedTriple,
        _ => Technique.NakedQuad,
    };

    private static Technique HiddenTechnique(int size) => size switch
    {
        2 => Technique.HiddenPair,
        3 => Technique.HiddenTriple,
        _ => Technique.HiddenQuad,
    };
}
