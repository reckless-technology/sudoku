using Xunit;

namespace Sudoku.Tests;

/// <summary>
/// Every puzzle in <c>puzzles/</c>: one solution by search, and the solver reaching it by deduction - every value it
/// places the solution's, every candidate it strikes not the solution's - with every technique used somewhere.
/// </summary>
public class PuzzleTests
{
    private static readonly string Folder = Path.Combine(AppContext.BaseDirectory, "puzzles");

    /// <summary>The puzzles' file names.</summary>
    public static TheoryData<string> Puzzles()
    {
        var names = new TheoryData<string>();
        foreach (string path in Directory.GetFiles(Folder).Order())
        {
            names.Add(Path.GetFileName(path));
        }
        return names;
    }

    [Fact]
    public void ThePuzzlesAreThere()
    {
        Assert.Equal(68, Puzzles().Count);
    }

    [Theory]
    [MemberData(nameof(Puzzles))]
    public void EachPuzzleIsSolvedByDeductionAndEveryStepIsTrue(string name)
    {
        int[] givens = PuzzleFile.Read(Path.Combine(Folder, name));
        Assert.Equal(SolutionCount.One, BruteForce.Count(givens, out int[]? solution));
        Assert.NotNull(solution);

        var solver = new Solver(Grid.FromValues(givens));
        while (solver.NextStep() is { } step)
        {
            foreach (Candidate placement in step.Placements)
            {
                Assert.True(solution[placement.Cell] == placement.Digit, $"{name}: {step.Description} places {placement}");
            }
            foreach (Candidate elimination in step.Eliminations)
            {
                Assert.True(solution[elimination.Cell] != elimination.Digit,
                            $"{name}: {step.Description} strikes the solution's {elimination}");
            }
            Assert.False(string.IsNullOrWhiteSpace(step.Description));
            Assert.NotEmpty(step.Pattern);
        }
        Assert.True(solver.Grid.IsComplete, $"{name}: stuck with {solver.Grid.Values().Count(v => v == 0)} squares empty");
        Assert.Equal(solution, solver.Grid.Values());
    }

    [Fact]
    public void EveryTechniqueIsUsedOnSomePuzzle()
    {
        var used = new HashSet<Technique>();
        foreach (string path in Directory.GetFiles(Folder))
        {
            used.UnionWith(new Solver(Grid.FromValues(PuzzleFile.Read(path))).Solve().Select(step => step.Technique));
        }
        Technique[] missing = Enum.GetValues<Technique>().Except(UnusedByThePuzzles).Except(used).ToArray();
        Assert.True(missing.Length == 0, "no puzzle uses " + string.Join(", ", missing));
        Assert.Empty(UnusedByThePuzzles.Intersect(used));
    }

    /// <summary>The techniques no puzzle here calls for, each held to its own suite instead (TechniqueTests).</summary>
    private static readonly Technique[] UnusedByThePuzzles =
        [Technique.NakedQuad, Technique.Jellyfish, Technique.ForcingChain, Technique.ContradictionChain];
}
