using System.Text;

namespace Sudoku;

/// <summary>
/// Puzzles in files. A <c>.sud</c> file is 81 bytes, one per square row by row from the top left, 0 for an empty
/// square. Any other file is text: the 81 squares row by row, a digit or <c>.</c> (or <c>0</c>) for an empty square,
/// whatever else it holds - line breaks, spaces - ignored; a <c>.sdk</c> file is nine lines of nine.
/// </summary>
public static class PuzzleFile
{
    /// <param name="path">The file.</param>
    /// <returns>Its 81 values, 0 for an empty square.</returns>
    /// <exception cref="FormatException">The file is not a puzzle.</exception>
    public static int[] Read(string path)
    {
        byte[] bytes = File.ReadAllBytes(path);
        return IsBinary(path) ? FromBytes(bytes) : Parse(Encoding.UTF8.GetString(bytes));
    }

    /// <summary>Writes a puzzle, as bytes for a <c>.sud</c> file and as nine lines of nine for any other.</summary>
    /// <param name="path">The file.</param>
    /// <param name="values">81 values, 0 for an empty square.</param>
    public static void Write(string path, IReadOnlyList<int> values)
    {
        if (IsBinary(path))
        {
            File.WriteAllBytes(path, values.Select(value => (byte)value).ToArray());
        }
        else
        {
            File.WriteAllText(path, Format(values));
        }
    }

    /// <param name="text">A puzzle as text.</param>
    /// <returns>Its 81 values, 0 for an empty square.</returns>
    /// <exception cref="FormatException">The text does not hold 81 squares.</exception>
    public static int[] Parse(string text)
    {
        int[] values = text.Where(c => c == '.' || c is >= '0' and <= '9')
                           .Select(c => c == '.' ? 0 : c - '0')
                           .ToArray();
        if (values.Length != Geometry.CellCount)
        {
            throw new FormatException($"a puzzle has {Geometry.CellCount} squares; this text has {values.Length}");
        }
        return values;
    }

    /// <param name="values">81 values, 0 for an empty square.</param>
    /// <returns>The puzzle as nine lines of nine, <c>.</c> for an empty square.</returns>
    public static string Format(IReadOnlyList<int> values)
    {
        var text = new StringBuilder();
        for (int row = 0; row < 9; row++)
        {
            for (int column = 0; column < 9; column++)
            {
                int value = values[Geometry.Cell(row, column)];
                text.Append(value == 0 ? '.' : (char)('0' + value));
            }
            text.Append("\r\n");
        }
        return text.ToString();
    }

    private static bool IsBinary(string path) => Path.GetExtension(path).Equals(".sud", StringComparison.OrdinalIgnoreCase);

    private static int[] FromBytes(byte[] bytes)
    {
        if (bytes.Length != Geometry.CellCount || bytes.Any(b => b > 9))
        {
            throw new FormatException($"a .sud file is {Geometry.CellCount} bytes of 0 to 9");
        }
        return bytes.Select(b => (int)b).ToArray();
    }
}
