using Sudoku.Techniques;
using Xunit;

namespace Sudoku.Tests;

/// <summary>
/// Each technique on its own: called directly on every puzzle as it is solved, whatever it finds must be true - so a
/// technique the solver's order rarely reaches is held to the solution all the same - and found where a pattern is
/// built for it.
/// </summary>
public class TechniqueTests
{
    private static readonly string Folder = Path.Combine(AppContext.BaseDirectory, "puzzles");

    /// <summary>The techniques, by name, with how often to try the slow ones (every n-th board on the way).</summary>
    public static TheoryData<string> Finders() => new(Finder.All.Keys);

    [Theory]
    [MemberData(nameof(Finders))]
    public void WhatATechniqueFindsAnywhereIsTrue(string name)
    {
        (Solver.Finder finder, int every) = Finder.All[name];
        int found = 0;
        foreach (string path in Directory.GetFiles(Folder).Order())
        {
            int[] givens = PuzzleFile.Read(path);
            BruteForce.Count(givens, out int[]? solution);
            Assert.NotNull(solution);
            var solver = new Solver(Grid.FromValues(givens));
            int board = 0;
            do
            {
                if (board++ % every != 0)
                {
                    continue;
                }
                Grid before = solver.Grid.Clone();
                if (finder(before) is not { } step)
                {
                    continue;
                }
                found++;
                Assert.Equal(solver.Grid.Values(), before.Values()); // finding changes nothing
                foreach (Candidate placement in step.Placements)
                {
                    Assert.True(solution[placement.Cell] == placement.Digit, $"{Path.GetFileName(path)}: {step}");
                    Assert.True(solver.Grid.IsEmpty(placement.Cell), $"{Path.GetFileName(path)}: {step} fills a full square");
                }
                foreach (Candidate elimination in step.Eliminations)
                {
                    Assert.True(solution[elimination.Cell] != elimination.Digit, $"{Path.GetFileName(path)}: {step}");
                    Assert.True(Digits.Contains(solver.Grid.Candidates(elimination.Cell), elimination.Digit),
                                $"{Path.GetFileName(path)}: {step} strikes what is already gone");
                }
                Assert.True(step.IsPlacement || step.Eliminations.Count > 0, $"{step} does nothing");
            } while (solver.NextStep() is not null);
        }
        Assert.True(found > 0, $"{name} found nothing on any puzzle");
    }

    [Fact]
    public void ANakedQuadNeedNotHaveEveryDigitInEverySquare()
    {
        var grid = new Grid();
        // Row 1's first four squares: {1,2}, {2,3}, {3,4}, {1,4} - no pair or triple among them, a quad together.
        Restrict(grid, 0, 1, 2);
        Restrict(grid, 1, 2, 3);
        Restrict(grid, 2, 3, 4);
        Restrict(grid, 3, 1, 4);
        Assert.Null(Subsets.Naked(grid, 2));
        Assert.Null(Subsets.Naked(grid, 3));
        Step? step = Subsets.Quads(grid);
        Assert.NotNull(step);
        Assert.Equal(Technique.NakedQuad, step.Technique);
        Assert.Equal([0, 1, 2, 3], step.Pattern);
        Assert.Equal(Enumerable.Range(4, 5).SelectMany(cell => Enumerable.Range(1, 4).Select(d => new Candidate(cell, d))),
                     step.Eliminations.OrderBy(c => c.Cell).ThenBy(c => c.Digit));
    }

    [Fact]
    public void ANakedTripleIsFoundWithNoSquareHoldingAllThree()
    {
        var grid = new Grid();
        Restrict(grid, 0, 1, 2);
        Restrict(grid, 1, 2, 3);
        Restrict(grid, 2, 1, 3);
        Step? step = Subsets.Naked(grid, 3);
        Assert.NotNull(step);
        Assert.Equal(Technique.NakedTriple, step.Technique);
        Assert.Contains(new Candidate(8, 1), step.Eliminations);
    }

    [Fact]
    public void AJellyfishStrikesTheDigitFromItsColumnsOutsideItsRows()
    {
        var grid = new Grid();
        // 1 only in columns 1 to 4 on rows 1 to 4.
        for (int row = 0; row < 4; row++)
        {
            for (int column = 4; column < 9; column++)
            {
                grid.Eliminate(Geometry.Cell(row, column), 1);
            }
        }
        Assert.Null(Fish.XWing(grid));
        Assert.Null(Fish.Swordfish(grid));
        Step? step = Fish.Jellyfish(grid);
        Assert.NotNull(step);
        Assert.Equal(Technique.Jellyfish, step.Technique);
        var expected = Enumerable.Range(4, 5).SelectMany(row => Enumerable.Range(0, 4).Select(c => Geometry.Cell(row, c)))
                           .Select(cell => new Candidate(cell, 1));
        Assert.Equal(expected.OrderBy(c => c.Cell), step.Eliminations.OrderBy(c => c.Cell));
    }

    [Fact]
    public void AnXWingStrikesTheDigitFromItsColumnsOutsideItsRows()
    {
        var grid = new Grid();
        // 5 only in columns 2 and 7 on rows 3 and 8.
        foreach (int row in new[] { 2, 7 })
        {
            for (int column = 0; column < 9; column++)
            {
                if (column != 1 && column != 6)
                {
                    grid.Eliminate(Geometry.Cell(row, column), 5);
                }
            }
        }
        Step? step = Fish.XWing(grid);
        Assert.NotNull(step);
        Assert.Equal(Technique.XWing, step.Technique);
        Assert.Equal(14, step.Eliminations.Count);
        Assert.All(step.Eliminations, c => Assert.True(Geometry.Column(c.Cell) is 1 or 6 && Geometry.Row(c.Cell) is not (2 or 7)));
    }

    [Fact]
    public void TheCombinationsAreEveryChoiceOnceInOrder()
    {
        Assert.Equal([[1, 2], [1, 3], [1, 4], [2, 3], [2, 4], [3, 4]], Combinations.Of([1, 2, 3, 4], 2));
        Assert.Empty(Combinations.Of([1, 2], 3));
    }

    /// <summary>A square's candidates cut to the digits given.</summary>
    private static void Restrict(Grid grid, int cell, params int[] digits)
    {
        foreach (int digit in Enumerable.Range(1, 9).Except(digits))
        {
            grid.Eliminate(cell, digit);
        }
    }

    /// <summary>Every technique, by name; the forcing ones tried on every eighth board, they being slow.</summary>
    private static class Finder
    {
        public static readonly Dictionary<string, (Solver.Finder Find, int Every)> All = new()
        {
            ["NakedSingle"] = (Singles.Naked, 1),
            ["HiddenSingle"] = (Singles.Hidden, 1),
            ["NakedPair"] = (grid => Subsets.Naked(grid, 2), 1),
            ["HiddenPair"] = (grid => Subsets.Hidden(grid, 2), 1),
            ["Pointing"] = (Intersections.Pointing, 1),
            ["Claiming"] = (Intersections.Claiming, 1),
            ["NakedTriple"] = (grid => Subsets.Naked(grid, 3), 1),
            ["HiddenTriple"] = (grid => Subsets.Hidden(grid, 3), 1),
            ["NakedQuad"] = (grid => Subsets.Naked(grid, 4), 1),
            ["HiddenQuad"] = (grid => Subsets.Hidden(grid, 4), 1),
            ["XWing"] = (Fish.XWing, 1),
            ["Swordfish"] = (Fish.Swordfish, 1),
            ["Jellyfish"] = (Fish.Jellyfish, 1),
            ["XYWing"] = (Wings.XY, 1),
            ["XYZWing"] = (Wings.XYZ, 1),
            ["SimpleColouring"] = (Chains.SimpleColouring, 1),
            ["XYChain"] = (Chains.XYChain, 1),
            ["Nishio"] = (Forcing.Nishio, 4),
            ["ForcingChain"] = (Forcing.ForcingChain, 8),
            ["ContradictionChain"] = (Forcing.ContradictionChain, 8),
        };
    }
}
