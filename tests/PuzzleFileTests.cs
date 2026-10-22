using Xunit;

namespace Sudoku.Tests;

/// <summary>Puzzles in files: 81 bytes in a .sud file, text in any other, each read back as written.</summary>
public class PuzzleFileTests
{
    private static readonly int[] Puzzle = PuzzleFile.Parse("""
        ...13...5
        .4....2..
        8..9.....
        ....5.9..
        ..2...4..
        ..3.6....
        .....3..6
        ..5....1.
        7...28...
        """);

    [Theory]
    [InlineData(".sud")]
    [InlineData(".sdk")]
    [InlineData(".txt")]
    public void APuzzleReadsBackAsItWasWritten(string extension)
    {
        string path = Path.Combine(Path.GetTempPath(), $"sudoku-{Guid.NewGuid():N}{extension}");
        try
        {
            PuzzleFile.Write(path, Puzzle);
            Assert.Equal(Puzzle, PuzzleFile.Read(path));
            Assert.Equal(extension == ".sud" ? 81 : 99, new FileInfo(path).Length);
        }
        finally
        {
            File.Delete(path);
        }
    }

    [Fact]
    public void TextTakesDotsOrZerosAndIgnoresEverythingElse()
    {
        string zeros = string.Join(" | ", PuzzleFile.Format(Puzzle).Replace('.', '0').Split("\r\n"));
        Assert.Equal(Puzzle, PuzzleFile.Parse(zeros));
        Assert.Equal(".", PuzzleFile.Format(Puzzle)[..1]);
    }

    [Fact]
    public void AFileThatIsNotAPuzzleIsRefused()
    {
        Assert.Throws<FormatException>(() => PuzzleFile.Parse("123"));
        string path = Path.Combine(Path.GetTempPath(), $"sudoku-{Guid.NewGuid():N}.sud");
        try
        {
            File.WriteAllBytes(path, new byte[80]);
            Assert.Throws<FormatException>(() => PuzzleFile.Read(path));
            File.WriteAllBytes(path, Enumerable.Repeat((byte)10, 81).ToArray());
            Assert.Throws<FormatException>(() => PuzzleFile.Read(path));
            File.WriteAllBytes(path, new byte[82]);
            Assert.Throws<FormatException>(() => PuzzleFile.Read(path));
        }
        finally
        {
            File.Delete(path);
        }
    }

    [Fact]
    public void TheSudFileAndTheSdkFileOfTheSamePuzzleAgree()
    {
        string folder = Path.Combine(AppContext.BaseDirectory, "puzzles");
        Assert.Equal(PuzzleFile.Read(Path.Combine(folder, "xwing1.sdk")), PuzzleFile.Read(Path.Combine(folder, "xwing1.sud")));
    }
}
