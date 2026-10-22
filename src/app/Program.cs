namespace Sudoku;

internal static class Program
{
    /// <summary>The window, with the puzzle named on the command line open in it.</summary>
    [STAThread]
    private static void Main(string[] args)
    {
        ApplicationConfiguration.Initialize();
        using var form = new MainForm();
        if (args.Length > 0)
        {
            form.OpenFile(args[0]);
        }
        Application.Run(form);
    }
}
