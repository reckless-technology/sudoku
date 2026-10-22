namespace Sudoku;

/// <summary>What kind of unit a row, a column or a box is.</summary>
public enum UnitKind
{
    /// <summary>A row, left to right.</summary>
    Row,

    /// <summary>A column, top to bottom.</summary>
    Column,

    /// <summary>A 3 x 3 box, numbered left to right and top to bottom.</summary>
    Box,
}

/// <summary>Nine squares that must hold each digit once: a row, a column or a box.</summary>
public sealed class Unit
{
    internal Unit(UnitKind kind, int index, int[] cells)
    {
        Kind = kind;
        Index = index;
        Cells = cells;
    }

    /// <summary>A row, a column or a box.</summary>
    public UnitKind Kind { get; }

    /// <summary>Which one, from 0.</summary>
    public int Index { get; }

    /// <summary>Its nine squares, in reading order.</summary>
    public IReadOnlyList<int> Cells { get; }

    /// <returns>Its name: "row 4", "column 7", "box 5".</returns>
    public override string ToString() => $"{Kind.ToString().ToLowerInvariant()} {Index + 1}";
}

/// <summary>
/// The board's shape: 81 squares numbered row by row from the top left (square <c>column + 9 * row</c>), its 27 units,
/// and the 20 squares each square shares a unit with.
/// </summary>
public static class Geometry
{
    /// <summary>The squares on the board.</summary>
    public const int CellCount = 81;

    /// <summary>The rows, top to bottom.</summary>
    public static readonly IReadOnlyList<Unit> Rows;

    /// <summary>The columns, left to right.</summary>
    public static readonly IReadOnlyList<Unit> Columns;

    /// <summary>The boxes, left to right and top to bottom.</summary>
    public static readonly IReadOnlyList<Unit> Boxes;

    /// <summary>Every unit: the rows, then the columns, then the boxes.</summary>
    public static readonly IReadOnlyList<Unit> Units;

    private static readonly int[][] peers;

    static Geometry()
    {
        Rows = Enumerable.Range(0, 9)
                   .Select(row => new Unit(UnitKind.Row, row, Enumerable.Range(0, 9).Select(c => Cell(row, c)).ToArray()))
                   .ToArray();
        Columns = Enumerable.Range(0, 9)
                      .Select(column => new Unit(UnitKind.Column, column,
                                                 Enumerable.Range(0, 9).Select(r => Cell(r, column)).ToArray()))
                      .ToArray();
        Boxes = Enumerable.Range(0, 9)
                    .Select(box => new Unit(UnitKind.Box, box,
                                            Enumerable.Range(0, 9)
                                                .Select(i => Cell(box / 3 * 3 + i / 3, box % 3 * 3 + i % 3))
                                                .ToArray()))
                    .ToArray();
        Units = Rows.Concat(Columns).Concat(Boxes).ToArray();
        peers = Enumerable.Range(0, CellCount)
                    .Select(cell => Enumerable.Range(0, CellCount).Where(other => Sees(cell, other)).ToArray())
                    .ToArray();
    }

    /// <param name="cell">A square.</param>
    /// <returns>Its row, from 0.</returns>
    public static int Row(int cell) => cell / 9;

    /// <param name="cell">A square.</param>
    /// <returns>Its column, from 0.</returns>
    public static int Column(int cell) => cell % 9;

    /// <param name="cell">A square.</param>
    /// <returns>Its box, from 0.</returns>
    public static int Box(int cell) => cell / 27 * 3 + cell % 9 / 3;

    /// <param name="row">A row, from 0.</param>
    /// <param name="column">A column, from 0.</param>
    /// <returns>The square where they cross.</returns>
    public static int Cell(int row, int column) => row * 9 + column;

    /// <param name="cell">A square.</param>
    /// <returns>Its name, row and column from 1: "r4c7".</returns>
    public static string Name(int cell) => $"r{Row(cell) + 1}c{Column(cell) + 1}";

    /// <param name="cells">Squares.</param>
    /// <returns>Their names, joined: "r4c2, r4c6".</returns>
    public static string Names(IEnumerable<int> cells) => string.Join(", ", cells.Select(Name));

    /// <param name="a">A square.</param>
    /// <param name="b">Another square.</param>
    /// <returns>Whether the two are different squares that share a unit, so cannot hold the same digit.</returns>
    public static bool Sees(int a, int b) => a != b && (Row(a) == Row(b) || Column(a) == Column(b) || Box(a) == Box(b));

    /// <param name="cell">A square.</param>
    /// <returns>The 20 squares that share a unit with it.</returns>
    public static IReadOnlyList<int> Peers(int cell) => peers[cell];
}
