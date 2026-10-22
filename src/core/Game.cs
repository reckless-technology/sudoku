namespace Sudoku;

/// <summary>
/// The board a person works on: the values they entered (the givens), what the solver has deduced from them, the steps
/// it took, and the undo history. Every change to the givens rebuilds the board from the givens alone, so nothing
/// deduced from a value outlives the value: the deductions are dropped, and a clue or a solve finds them again.
/// </summary>
public sealed class Game
{
    private sealed record Snapshot(int[] Givens, Grid Grid, Step[] Steps);

    private readonly Stack<Snapshot> history = new();
    private readonly List<Step> steps = [];
    private int[] givens = new int[Geometry.CellCount];
    private Grid grid = new();

    /// <summary>An empty board.</summary>
    public Game()
    {
        Rebuild();
    }

    /// <summary>The givens, square <c>column + 9 * row</c>, 0 for an empty square.</summary>
    public IReadOnlyList<int> Givens => givens;

    /// <summary>The squares whose given clashes with another in one of its units, in order; empty for none.</summary>
    public IReadOnlyList<int> Clashes { get; private set; } = [];

    /// <summary>The steps the solver has taken since the givens last changed, oldest first.</summary>
    public IReadOnlyList<Step> Steps => steps;

    /// <summary>Whether there is a change to undo.</summary>
    public bool CanUndo => history.Count > 0;

    /// <summary>Whether every square has a value and the board holds no contradiction.</summary>
    public bool IsSolved => grid.IsComplete && grid.FindContradiction() is null;

    /// <summary>What makes the board impossible, in words; null while it is not.</summary>
    public string? Contradiction => grid.FindContradiction();

    /// <param name="cell">A square.</param>
    /// <returns>Its value, given or deduced; 0 while it is empty.</returns>
    public int Value(int cell) => givens[cell] != 0 ? givens[cell] : grid.Value(cell);

    /// <param name="cell">A square.</param>
    /// <returns>Whether its value was entered rather than deduced.</returns>
    public bool IsGiven(int cell) => givens[cell] != 0;

    /// <param name="cell">A square.</param>
    /// <returns>The digits it may still take, the pencil marks; none while the givens clash.</returns>
    public int Candidates(int cell) => Clashes.Count > 0 ? 0 : grid.Candidates(cell);

    /// <summary>Enters a given, or clears one, and rebuilds the board from the givens.</summary>
    /// <param name="cell">A square.</param>
    /// <param name="digit">Its value, 1 to 9; 0 to clear it.</param>
    /// <returns>Whether anything changed.</returns>
    public bool SetGiven(int cell, int digit)
    {
        if (digit < 0 || digit > 9)
        {
            throw new ArgumentOutOfRangeException(nameof(digit), digit, "a digit is 1 to 9, or 0 to clear");
        }
        if (givens[cell] == digit && (digit != 0 || grid.IsEmpty(cell)))
        {
            return false;
        }
        Remember();
        givens[cell] = digit;
        Rebuild();
        return true;
    }

    /// <summary>A puzzle in place of the board.</summary>
    /// <param name="values">81 values, 0 for an empty square.</param>
    public void Load(IReadOnlyList<int> values)
    {
        if (values.Count != Geometry.CellCount || values.Any(value => value < 0 || value > 9))
        {
            throw new ArgumentException("a puzzle is 81 values of 0 to 9", nameof(values));
        }
        Remember();
        givens = values.ToArray();
        Rebuild();
    }

    /// <summary>Every square emptied.</summary>
    public void Clear() => Load(new int[Geometry.CellCount]);

    /// <summary>Steps until one places a value.</summary>
    /// <returns>The steps taken; empty when there was nothing to find, or the givens clash.</returns>
    public IReadOnlyList<Step> Clue() => Run(solver => solver.Clue());

    /// <summary>Steps until the board is complete or nothing more can be found.</summary>
    /// <returns>The steps taken; empty when there was nothing to find, or the givens clash.</returns>
    public IReadOnlyList<Step> Solve() => Run(solver => solver.Solve());

    /// <summary>How many solutions the givens have, by search.</summary>
    /// <param name="solution">The solution, where there is one or more.</param>
    public SolutionCount CountSolutions(out int[]? solution) => BruteForce.Count(givens, out solution);

    /// <summary>The last change - a given, a load, a clear, a clue or a solve - taken back.</summary>
    /// <returns>Whether there was one.</returns>
    public bool Undo()
    {
        if (!history.TryPop(out Snapshot? snapshot))
        {
            return false;
        }
        givens = snapshot.Givens;
        grid = snapshot.Grid;
        steps.Clear();
        steps.AddRange(snapshot.Steps);
        Clashes = Grid.Clashes(givens);
        return true;
    }

    private IReadOnlyList<Step> Run(Func<Solver, IReadOnlyList<Step>> run)
    {
        if (Clashes.Count > 0)
        {
            return [];
        }
        var before = new Snapshot(givens.ToArray(), grid.Clone(), steps.ToArray());
        IReadOnlyList<Step> taken = run(new Solver(grid));
        if (taken.Count > 0)
        {
            history.Push(before);
            steps.AddRange(taken);
        }
        return taken;
    }

    private void Remember() => history.Push(new Snapshot(givens.ToArray(), grid.Clone(), steps.ToArray()));

    private void Rebuild()
    {
        Clashes = Grid.Clashes(givens);
        grid = Grid.FromValues(givens);
        steps.Clear();
    }
}
