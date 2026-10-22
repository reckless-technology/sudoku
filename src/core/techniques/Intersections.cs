namespace Sudoku.Techniques;

/// <summary>
/// Where a box and a line cross. Pointing: a digit whose places in a box all lie on one row or column must be in the box
/// there, so the rest of the line loses it. Claiming: a digit whose places on a line all lie in one box must be on the
/// line there, so the rest of the box loses it.
/// </summary>
public static class Intersections
{
    /// <summary>The first pointing digit that strikes a candidate, then the first claiming one.</summary>
    public static Step? Find(Grid grid) => Pointing(grid) ?? Claiming(grid);

    /// <summary>The first pointing digit that strikes a candidate.</summary>
    public static Step? Pointing(Grid grid)
    {
        foreach (Unit box in Geometry.Boxes)
        {
            for (int digit = 1; digit <= 9; digit++)
            {
                List<int> places = grid.PlacesFor(box, digit);
                if (places.Count < 2 || grid.HasPlaced(box, digit))
                {
                    continue;
                }
                foreach (Unit line in LinesThrough(places))
                {
                    var eliminations = line.Cells.Where(cell => Geometry.Box(cell) != box.Index)
                                           .Where(cell => grid.IsEmpty(cell) && Digits.Contains(grid.Candidates(cell), digit))
                                           .Select(cell => new Candidate(cell, digit))
                                           .ToList();
                    if (eliminations.Count > 0)
                    {
                        return new Step(Technique.Pointing,
                                        $"Pointing: {digit} in {box} lies only in {line} ({Geometry.Names(places)}), so the " +
                                            $"rest of {line} {TechniqueNames.Removes(eliminations)}",
                                        [], eliminations, places);
                    }
                }
            }
        }
        return null;
    }

    /// <summary>The first claiming digit that strikes a candidate.</summary>
    public static Step? Claiming(Grid grid)
    {
        foreach (Unit line in Geometry.Rows.Concat(Geometry.Columns))
        {
            for (int digit = 1; digit <= 9; digit++)
            {
                List<int> places = grid.PlacesFor(line, digit);
                if (places.Count < 2 || grid.HasPlaced(line, digit) ||
                    places.Select(Geometry.Box).Distinct().Count() != 1)
                {
                    continue;
                }
                Unit box = Geometry.Boxes[Geometry.Box(places[0])];
                var eliminations = box.Cells.Where(cell => !line.Cells.Contains(cell))
                                       .Where(cell => grid.IsEmpty(cell) && Digits.Contains(grid.Candidates(cell), digit))
                                       .Select(cell => new Candidate(cell, digit))
                                       .ToList();
                if (eliminations.Count > 0)
                {
                    return new Step(Technique.Claiming,
                                    $"Claiming: {digit} on {line} lies only in {box} ({Geometry.Names(places)}), so the " +
                                        $"rest of {box} {TechniqueNames.Removes(eliminations)}",
                                    [], eliminations, places);
                }
            }
        }
        return null;
    }

    /// <returns>The row and the column all the squares share, where they share one.</returns>
    private static IEnumerable<Unit> LinesThrough(IReadOnlyList<int> cells)
    {
        if (cells.Select(Geometry.Row).Distinct().Count() == 1)
        {
            yield return Geometry.Rows[Geometry.Row(cells[0])];
        }
        if (cells.Select(Geometry.Column).Distinct().Count() == 1)
        {
            yield return Geometry.Columns[Geometry.Column(cells[0])];
        }
    }
}
