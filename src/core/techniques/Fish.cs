namespace Sudoku.Techniques;

/// <summary>
/// X-wing, swordfish and jellyfish: a digit whose places in n rows all lie in the same n columns must take one place in
/// each of those columns from those rows, so the columns' other squares lose it - and the same with the rows and the
/// columns exchanged. A naked subset of one digit across lines.
/// </summary>
public static class Fish
{
    /// <summary>The first X-wing (two lines) that strikes a candidate.</summary>
    public static Step? XWing(Grid grid) => Find(grid, 2, Technique.XWing);

    /// <summary>The first swordfish (three lines) that strikes a candidate.</summary>
    public static Step? Swordfish(Grid grid) => Find(grid, 3, Technique.Swordfish);

    /// <summary>The first jellyfish (four lines) that strikes a candidate.</summary>
    public static Step? Jellyfish(Grid grid) => Find(grid, 4, Technique.Jellyfish);

    private static Step? Find(Grid grid, int size, Technique technique)
    {
        for (int digit = 1; digit <= 9; digit++)
        {
            foreach (bool isRowBased in new[] { true, false })
            {
                IReadOnlyList<Unit> bases = isRowBased ? Geometry.Rows : Geometry.Columns;
                IReadOnlyList<Unit> covers = isRowBased ? Geometry.Columns : Geometry.Rows;
                // Each base line's places for the digit, as a set of cover lines (a bit per cover line).
                var candidates = new List<(Unit Line, int Covers, List<int> Places)>();
                foreach (Unit line in bases)
                {
                    if (grid.HasPlaced(line, digit))
                    {
                        continue;
                    }
                    List<int> places = grid.PlacesFor(line, digit);
                    if (places.Count >= 2 && places.Count <= size)
                    {
                        int coverSet = places.Aggregate(0, (set, cell) => set | 1 << CoverIndex(cell, isRowBased));
                        candidates.Add((line, coverSet, places));
                    }
                }
                foreach (var fish in Combinations.Of(candidates, size))
                {
                    int coverSet = fish.Aggregate(0, (set, line) => set | line.Covers);
                    if (Digits.Count(coverSet) != size)
                    {
                        continue;
                    }
                    var baseIndices = fish.Select(line => line.Line.Index).ToHashSet();
                    var eliminations = new List<Candidate>();
                    for (int cover = 0; cover < 9; cover++)
                    {
                        if ((coverSet & 1 << cover) == 0)
                        {
                            continue;
                        }
                        eliminations.AddRange(grid.PlacesFor(covers[cover], digit)
                                                  .Where(cell => !baseIndices.Contains(BaseIndex(cell, isRowBased)))
                                                  .Select(cell => new Candidate(cell, digit)));
                    }
                    if (eliminations.Count > 0)
                    {
                        int[] pattern = fish.SelectMany(line => line.Places).Order().ToArray();
                        string baseNames = string.Join(", ", fish.Select(line => line.Line.ToString()));
                        string coverNames = string.Join(", ", Enumerable.Range(0, 9)
                                                                  .Where(cover => (coverSet & 1 << cover) != 0)
                                                                  .Select(cover => covers[cover].ToString()));
                        return new Step(technique,
                                        $"{technique.Name()}: {digit} in {baseNames} lies only in {coverNames} " +
                                            $"({Geometry.Names(pattern)}), so the rest of those lines " +
                                            TechniqueNames.Removes(eliminations),
                                        [], eliminations, pattern);
                    }
                }
            }
        }
        return null;
    }

    private static int CoverIndex(int cell, bool isRowBased) => isRowBased ? Geometry.Column(cell) : Geometry.Row(cell);

    private static int BaseIndex(int cell, bool isRowBased) => isRowBased ? Geometry.Row(cell) : Geometry.Column(cell);
}
