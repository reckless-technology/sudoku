using Xunit;

namespace Sudoku.Tests;

/// <summary>
/// The board a person works on: nothing deduced from a value outlives it, clashing givens found before the solver runs,
/// every change undone, a puzzle's solutions counted.
/// </summary>
public class GameTests
{
    private static readonly string Folder = Path.Combine(AppContext.BaseDirectory, "puzzles");

    [Fact]
    public void ClearingAGivenGivesBackTheCandidatesItStruck()
    {
        var game = new Game();
        game.SetGiven(0, 5);
        Assert.False(Digits.Contains(game.Candidates(1), 5));
        game.Clue();
        game.SetGiven(0, 0);
        for (int cell = 1; cell < Geometry.CellCount; cell++)
        {
            Assert.Equal(Digits.All, game.Candidates(cell));
        }
        Assert.Empty(game.Steps);
    }

    [Fact]
    public void ChangingAGivenDropsWhatWasDeducedFromTheOldOne()
    {
        var game = new Game();
        game.Load(PuzzleFile.Read(Path.Combine(Folder, "easy1.sud")));
        Assert.NotEmpty(game.Solve());
        Assert.True(game.IsSolved);
        int empty = Enumerable.Range(0, Geometry.CellCount).First(cell => !game.IsGiven(cell));
        int other = Enumerable.Range(1, 9).First(digit => digit != game.Value(empty));
        game.SetGiven(empty, other);
        Assert.Empty(game.Steps);
        Assert.All(Enumerable.Range(0, Geometry.CellCount).Where(cell => !game.IsGiven(cell)),
                   cell => Assert.Equal(0, game.Value(cell)));
    }

    [Fact]
    public void ClashingGivensAreNamedAndTheSolverWaitsForThem()
    {
        var game = new Game();
        game.SetGiven(Geometry.Cell(0, 0), 5);
        game.SetGiven(Geometry.Cell(0, 8), 5);
        Assert.Equal([Geometry.Cell(0, 0), Geometry.Cell(0, 8)], game.Clashes);
        Assert.Equal("5 is twice in row 1 (r1c1, r1c9)", game.Contradiction);
        Assert.Empty(game.Clue());
        Assert.Empty(game.Solve());
        Assert.Equal(0, game.Candidates(Geometry.Cell(4, 4)));
        game.SetGiven(Geometry.Cell(0, 8), 0);
        Assert.Empty(game.Clashes);
        Assert.Null(game.Contradiction);
    }

    [Fact]
    public void GivensThatClashWithNothingButLeaveNoWayAreNamedToo()
    {
        var game = new Game();
        // Row 1 holds 1 to 8 and r2c9 is 9: r1c9 can be nothing.
        for (int column = 0; column < 8; column++)
        {
            game.SetGiven(Geometry.Cell(0, column), column + 1);
        }
        game.SetGiven(Geometry.Cell(1, 8), 9);
        Assert.Empty(game.Clashes);
        Assert.Equal("r1c9 has no digit left", game.Contradiction);
        Assert.Equal(SolutionCount.None, game.CountSolutions(out _));
    }

    [Fact]
    public void UndoTakesBackEachChangeInTurn()
    {
        var game = new Game();
        Assert.False(game.CanUndo);
        game.Load(PuzzleFile.Read(Path.Combine(Folder, "medium1.sud")));
        int[] puzzle = game.Givens.ToArray();
        IReadOnlyList<Step> clue = game.Clue();
        Assert.True(clue[^1].IsPlacement);
        Candidate placed = clue[^1].Placements[0];
        Assert.Equal(placed.Digit, game.Value(placed.Cell));
        game.SetGiven(placed.Cell, 0);
        Assert.Equal(0, game.Value(placed.Cell));
        Assert.False(game.SetGiven(placed.Cell, 0)); // nothing to change: no undo for it

        Assert.True(game.Undo()); // the clearing
        Assert.Equal(placed.Digit, game.Value(placed.Cell));
        Assert.Equal(clue.Count, game.Steps.Count);
        Assert.True(game.Undo()); // the clue
        Assert.Equal(0, game.Value(placed.Cell));
        Assert.Empty(game.Steps);
        Assert.True(game.Undo()); // the load
        Assert.All(game.Givens, value => Assert.Equal(0, value));
        Assert.False(game.Undo());
        Assert.NotEqual(puzzle, game.Givens);
    }

    [Fact]
    public void ACluePlacesOneValueAndShowsTheStepsToIt()
    {
        var game = new Game();
        game.Load(PuzzleFile.Read(Path.Combine(Folder, "xwing1.sdk")));
        int before = Enumerable.Range(0, Geometry.CellCount).Count(cell => game.Value(cell) != 0);
        IReadOnlyList<Step> steps = game.Clue();
        Assert.NotEmpty(steps);
        Assert.True(steps[^1].IsPlacement);
        Assert.All(steps.Take(steps.Count - 1), step => Assert.False(step.IsPlacement));
        Assert.Equal(before + 1, Enumerable.Range(0, Geometry.CellCount).Count(cell => game.Value(cell) != 0));
    }

    [Fact]
    public void SolutionsAreCountedNoneOneOrMany()
    {
        Assert.Equal(SolutionCount.Many, BruteForce.Count(new int[Geometry.CellCount], out int[]? any));
        Assert.NotNull(any);
        Assert.Null(Grid.FromValues(any).FindContradiction());
        int[] puzzle = PuzzleFile.Read(Path.Combine(Folder, "hard1.sud"));
        Assert.Equal(SolutionCount.One, BruteForce.Count(puzzle, out int[]? solution));
        Assert.NotNull(solution);
        int empty = Array.IndexOf(puzzle, 0);
        puzzle[empty] = solution[empty] % 9 + 1; // a digit the one solution rules out
        Assert.Equal(SolutionCount.None, BruteForce.Count(puzzle, out _));
    }
}
