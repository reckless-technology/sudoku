namespace Sudoku.Techniques;

/// <summary>Simple colouring and XY-chains: reasoning along chains of squares, each link forcing the next.</summary>
public static class Chains
{
    /// <summary>
    /// Simple colouring. A digit with exactly two places in a unit is a strong link: one of the two holds it. The
    /// digit's strong links form chains, coloured alternately, so one colour holds the digit and the other does not.
    /// Two squares of one colour in the same unit make that colour false (a colour wrap); a square that sees both
    /// colours cannot hold the digit (a colour trap).
    /// </summary>
    public static Step? SimpleColouring(Grid grid)
    {
        for (int digit = 1; digit <= 9; digit++)
        {
            var links = new Dictionary<int, List<int>>();
            foreach (Unit unit in Geometry.Units)
            {
                List<int> places = grid.PlacesFor(unit, digit);
                if (places.Count == 2 && !grid.HasPlaced(unit, digit))
                {
                    Link(links, places[0], places[1]);
                    Link(links, places[1], places[0]);
                }
            }
            var colours = new Dictionary<int, int>();
            foreach (int start in links.Keys.Order())
            {
                if (colours.ContainsKey(start))
                {
                    continue;
                }
                var component = new List<int> { start };
                colours[start] = 0;
                var queue = new Queue<int>([start]);
                while (queue.TryDequeue(out int cell))
                {
                    foreach (int next in links[cell].Where(next => !colours.ContainsKey(next)))
                    {
                        colours[next] = 1 - colours[cell];
                        component.Add(next);
                        queue.Enqueue(next);
                    }
                }
                int[][] byColour = [component.Where(c => colours[c] == 0).Order().ToArray(),
                                    component.Where(c => colours[c] == 1).Order().ToArray()];
                for (int colour = 0; colour < 2; colour++)
                {
                    int[] same = byColour[colour];
                    if (Combinations.Of(same, 2).Any(pair => Geometry.Sees(pair[0], pair[1])))
                    {
                        var eliminations = same.Select(cell => new Candidate(cell, digit)).ToList();
                        return new Step(Technique.SimpleColouring,
                                        $"Simple colouring (wrap): {digit}'s chain {Geometry.Names(byColour[0])} | " +
                                            $"{Geometry.Names(byColour[1])} has two squares of one colour in a unit, so " +
                                            $"that colour is false - {TechniqueNames.Removes(eliminations)}",
                                        [], eliminations, component.Order().ToArray());
                    }
                }
                var trapped = new List<Candidate>();
                for (int cell = 0; cell < Geometry.CellCount; cell++)
                {
                    if (grid.IsEmpty(cell) && Digits.Contains(grid.Candidates(cell), digit) && !colours.ContainsKey(cell) &&
                        byColour[0].Any(c => Geometry.Sees(cell, c)) && byColour[1].Any(c => Geometry.Sees(cell, c)))
                    {
                        trapped.Add(new Candidate(cell, digit));
                    }
                }
                if (trapped.Count > 0)
                {
                    return new Step(Technique.SimpleColouring,
                                    $"Simple colouring (trap): {digit}'s chain {Geometry.Names(byColour[0])} | " +
                                        $"{Geometry.Names(byColour[1])} - one colour holds it, so what sees both colours " +
                                        $"loses it - {TechniqueNames.Removes(trapped)}",
                                    [], trapped, component.Order().ToArray());
                }
            }
        }
        return null;
    }

    /// <summary>
    /// XY-chain: a chain of two-candidate squares, each seeing the next and sharing a digit with it. If the first square
    /// is not z, it is its other digit, which the next square therefore is not, and so on to the last square, which
    /// is then z. So one end or the other is z, and a square seeing both ends cannot be.
    /// </summary>
    public static Step? XYChain(Grid grid)
    {
        int[] bivalue = Enumerable.Range(0, Geometry.CellCount)
                            .Where(cell => grid.IsEmpty(cell) && Digits.Count(grid.Candidates(cell)) == 2)
                            .ToArray();
        foreach (int start in bivalue)
        {
            foreach (int z in Digits.Each(grid.Candidates(start)))
            {
                // Breadth first, so the shortest chain is found: each square's digit if the start is not z.
                var holds = new Dictionary<int, int> { [start] = Digits.First(grid.Candidates(start) & ~Digits.Bit(z)) };
                var previous = new Dictionary<int, int>();
                var queue = new Queue<int>([start]);
                while (queue.TryDequeue(out int cell))
                {
                    int digit = holds[cell];
                    foreach (int next in bivalue.Where(next => !holds.ContainsKey(next) && Geometry.Sees(cell, next) &&
                                                               Digits.Contains(grid.Candidates(next), digit)))
                    {
                        holds[next] = Digits.First(grid.Candidates(next) & ~Digits.Bit(digit));
                        previous[next] = cell;
                        queue.Enqueue(next);
                        if (holds[next] != z)
                        {
                            continue;
                        }
                        var chain = new List<int> { next };
                        while (chain[^1] != start)
                        {
                            chain.Add(previous[chain[^1]]);
                        }
                        chain.Reverse();
                        if (chain.Count < 3)
                        {
                            continue; // two squares that see each other: a naked pair
                        }
                        var eliminations = Wings.Seeing(grid, z, [start, next], chain);
                        if (eliminations.Count > 0)
                        {
                            string links = string.Join(" - ", chain.Select(c => $"{Geometry.Name(c)}" +
                                                                                 Digits.Format(grid.Candidates(c))));
                            return new Step(Technique.XYChain,
                                            $"XY-chain {links}: one end is {z}, so what sees both ends loses it - " +
                                                TechniqueNames.Removes(eliminations),
                                            [], eliminations, chain);
                        }
                    }
                }
            }
        }
        return null;
    }

    private static void Link(Dictionary<int, List<int>> links, int from, int to)
    {
        if (!links.TryGetValue(from, out List<int>? list))
        {
            links[from] = list = [];
        }
        if (!list.Contains(to))
        {
            list.Add(to);
        }
    }
}
