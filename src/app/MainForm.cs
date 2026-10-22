namespace Sudoku;

/// <summary>
/// The window: the board, the solver's buttons, the steps it has taken (choosing one lights it on the board), and a
/// line saying where the puzzle stands - solved, stuck and why, or what makes it impossible.
/// </summary>
internal sealed class MainForm : Form
{
    private const string Title = "Jonny's Sudoku Solver";

    private readonly Game game = new();
    private readonly BoardView board = new() { Dock = DockStyle.Fill };
    private readonly ListBox stepList = new() { Dock = DockStyle.Fill, IntegralHeight = false, HorizontalScrollbar = true };
    private readonly Label status = new()
    {
        Dock = DockStyle.Bottom,
        AutoSize = false,
        AutoEllipsis = true,
        TextAlign = ContentAlignment.MiddleLeft,
        Padding = new Padding(6, 0, 6, 0),
        BorderStyle = BorderStyle.FixedSingle,
    };
    private readonly ToolStripMenuItem undoItem;
    private readonly ToolStripMenuItem clueItem;
    private readonly ToolStripMenuItem solveItem;
    private readonly ToolStripMenuItem pencilMarksItem;
    private readonly Button clueButton;
    private readonly Button solveButton;
    private readonly Button undoButton;
    private readonly ToolTip toolTip = new();
    private string? fileName;

    public MainForm()
    {
        Text = Title;
        AutoScaleMode = AutoScaleMode.Dpi;
        ClientSize = new Size(980, 640);
        MinimumSize = new Size(640, 460);
        StartPosition = FormStartPosition.CenterScreen;

        var menu = new MenuStrip();
        var fileMenu = new ToolStripMenuItem("&File");
        fileMenu.DropDownItems.AddRange([
            MenuItem("&Open...", Keys.Control | Keys.O, OpenPuzzle),
            MenuItem("&Save...", Keys.Control | Keys.S, SavePuzzle),
            new ToolStripSeparator(),
            MenuItem("E&xit", Keys.None, Close),
        ]);
        undoItem = MenuItem("&Undo", Keys.Control | Keys.Z, Undo);
        var editMenu = new ToolStripMenuItem("&Edit");
        editMenu.DropDownItems.AddRange([undoItem, MenuItem("&Clear board", Keys.None, ClearBoard)]);
        clueItem = MenuItem("C&lue", Keys.F5, Clue);
        solveItem = MenuItem("&Solve", Keys.F6, Solve);
        var puzzleMenu = new ToolStripMenuItem("&Puzzle");
        puzzleMenu.DropDownItems.AddRange([clueItem, solveItem, MenuItem("&Check solutions", Keys.F7, CheckSolutions)]);
        pencilMarksItem = MenuItem("&Pencil marks", Keys.Control | Keys.P, TogglePencilMarks);
        pencilMarksItem.Checked = true;
        var viewMenu = new ToolStripMenuItem("&View");
        viewMenu.DropDownItems.Add(pencilMarksItem);
        menu.Items.AddRange([fileMenu, editMenu, puzzleMenu, viewMenu]);
        MainMenuStrip = menu;

        clueButton = SideButton("Clue", Clue);
        solveButton = SideButton("Solve", Solve);
        undoButton = SideButton("Undo", Undo);
        var buttons = new FlowLayoutPanel { AutoSize = true, Dock = DockStyle.Fill, WrapContents = true };
        buttons.Controls.AddRange([clueButton, solveButton, SideButton("Check", CheckSolutions), undoButton,
                                   SideButton("Clear", ClearBoard)]);
        var side = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 1, RowCount = 3, Padding = new Padding(6) };
        side.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        side.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        side.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        side.Controls.Add(buttons, 0, 0);
        side.Controls.Add(new Label { Text = "Steps", AutoSize = true, Margin = new Padding(3, 10, 3, 3) }, 0, 1);
        side.Controls.Add(stepList, 0, 2);

        var layout = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 2, RowCount = 1 };
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 380));
        layout.Controls.Add(board, 0, 0);
        layout.Controls.Add(side, 1, 0);

        status.Height = (int)(26 * DeviceDpi / 96f);
        Controls.Add(layout);
        Controls.Add(status);
        Controls.Add(menu);

        board.Game = game;
        board.DigitEntered += EnterDigit;
        board.ContextMenuStrip = DigitMenu();
        stepList.SelectedIndexChanged += (_, _) =>
            board.Highlight = stepList.SelectedIndex >= 0 ? game.Steps[stepList.SelectedIndex] : null;
        Shown += (_, _) => board.Focus();
        ShowState();
    }

    /// <summary>A puzzle opened from a file, as File > Open does.</summary>
    /// <param name="path">The file.</param>
    public void OpenFile(string path)
    {
        try
        {
            game.Load(PuzzleFile.Read(path));
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException or FormatException)
        {
            MessageBox.Show(this, $"{Path.GetFileName(path)} is not a Sudoku puzzle: {exception.Message}.", Title,
                            MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }
        fileName = Path.GetFileName(path);
        ShowState($"Opened {fileName}.");
    }

    private void EnterDigit(int cell, int digit)
    {
        bool hadDeductions = game.Steps.Count > 0;
        if (!game.SetGiven(cell, digit))
        {
            return;
        }
        ShowState(hadDeductions ? "The givens changed, so the solver's values were cleared: Clue or Solve finds them again."
                                : null);
    }

    private void Clue()
    {
        IReadOnlyList<Step> steps = game.Clue();
        ShowState(steps.Count > 0 ? steps[^1].Description : WhyStuck());
    }

    private void Solve()
    {
        IReadOnlyList<Step> steps = game.Solve();
        ShowState(game.IsSolved ? $"Solved in {steps.Count} steps." : WhyStuck());
    }

    private void CheckSolutions()
    {
        string answer = game.CountSolutions(out _) switch
        {
            SolutionCount.None => "This puzzle has no solution.",
            SolutionCount.One => "This puzzle has exactly one solution.",
            _ => "This puzzle has more than one solution: it needs more givens.",
        };
        ShowState(answer);
    }

    private void Undo()
    {
        if (game.Undo())
        {
            ShowState("Undone.");
        }
    }

    private void ClearBoard()
    {
        game.Clear();
        fileName = null;
        ShowState("Board cleared.");
    }

    private void TogglePencilMarks()
    {
        board.ShowsPencilMarks = pencilMarksItem.Checked = !pencilMarksItem.Checked;
    }

    private void OpenPuzzle()
    {
        using var dialog = new OpenFileDialog
        {
            Title = "Open a puzzle",
            Filter = "Sudoku puzzles (*.sud;*.sdk)|*.sud;*.sdk|All files (*.*)|*.*",
        };
        if (dialog.ShowDialog(this) == DialogResult.OK)
        {
            OpenFile(dialog.FileName);
        }
    }

    private void SavePuzzle()
    {
        using var dialog = new SaveFileDialog
        {
            Title = "Save the puzzle's givens",
            Filter = "Sudoku board (*.sud)|*.sud|Sudoku text (*.sdk)|*.sdk",
            FileName = fileName ?? "puzzle.sud",
        };
        if (dialog.ShowDialog(this) != DialogResult.OK)
        {
            return;
        }
        try
        {
            PuzzleFile.Write(dialog.FileName, game.Givens);
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException)
        {
            MessageBox.Show(this, $"The puzzle was not saved: {exception.Message}", Title, MessageBoxButtons.OK,
                            MessageBoxIcon.Warning);
            return;
        }
        fileName = Path.GetFileName(dialog.FileName);
        ShowState($"Saved {fileName}.");
    }

    /// <returns>Why the solver stopped short: a contradiction, or the puzzle's solutions by search.</returns>
    private string WhyStuck()
    {
        if (game.Contradiction is { } contradiction)
        {
            return $"Impossible: {contradiction}.";
        }
        if (game.IsSolved)
        {
            return "Solved.";
        }
        return game.CountSolutions(out _) switch
        {
            SolutionCount.None => "Stuck: this puzzle has no solution.",
            SolutionCount.Many => "Stuck: this puzzle has more than one solution, so no deduction can finish it.",
            _ => "Stuck: the puzzle has one solution, but it needs a deeper chain than this solver follows.",
        };
    }

    /// <summary>The board, the steps, the buttons and the status line brought up to date.</summary>
    /// <param name="message">What to say; where the puzzle stands when null.</param>
    private void ShowState(string? message = null)
    {
        stepList.BeginUpdate();
        stepList.Items.Clear();
        for (int index = 0; index < game.Steps.Count; index++)
        {
            stepList.Items.Add($"{index + 1}. {game.Steps[index].Description}");
        }
        stepList.EndUpdate();
        if (stepList.Items.Count > 0)
        {
            stepList.SelectedIndex = stepList.Items.Count - 1;
        }
        else
        {
            board.Highlight = null;
        }
        bool canRun = game.Clashes.Count == 0 && game.Contradiction is null && !game.IsSolved;
        clueItem.Enabled = clueButton.Enabled = canRun;
        solveItem.Enabled = solveButton.Enabled = canRun;
        undoItem.Enabled = undoButton.Enabled = game.CanUndo;
        Text = fileName is null ? Title : $"{fileName} - {Title}";
        status.Text = message ?? Standing();
        toolTip.SetToolTip(status, status.Text);
        board.Invalidate();
    }

    /// <returns>Where the puzzle stands, in words.</returns>
    private string Standing()
    {
        if (game.Clashes.Count > 0)
        {
            return $"The givens clash: {game.Contradiction}.";
        }
        if (game.Contradiction is { } contradiction)
        {
            return $"Impossible: {contradiction}.";
        }
        if (game.IsSolved)
        {
            return "Solved.";
        }
        int filled = Enumerable.Range(0, Geometry.CellCount).Count(cell => game.Value(cell) != 0);
        return filled == 0 ? "Choose a square and type a digit, or open a puzzle." : $"{filled} of 81 squares filled.";
    }

    private ContextMenuStrip DigitMenu()
    {
        var digitMenu = new ContextMenuStrip();
        for (int digit = 1; digit <= 9; digit++)
        {
            digitMenu.Items.Add(new ToolStripMenuItem(digit.ToString()) { Tag = digit });
        }
        digitMenu.Items.Add(new ToolStripSeparator());
        digitMenu.Items.Add(new ToolStripMenuItem("Clear") { Tag = 0 });
        digitMenu.ItemClicked += (_, e) =>
        {
            if (e.ClickedItem?.Tag is int digit)
            {
                EnterDigit(board.Selected, digit);
            }
        };
        digitMenu.Opening += (_, e) => e.Cancel = board.CellAt(board.PointToClient(Cursor.Position)) < 0;
        return digitMenu;
    }

    private ToolStripMenuItem MenuItem(string text, Keys shortcut, Action action)
    {
        var item = new ToolStripMenuItem(text) { ShortcutKeys = shortcut };
        item.Click += (_, _) =>
        {
            action();
            board.Focus();
        };
        return item;
    }

    private Button SideButton(string text, Action action)
    {
        var button = new Button { Text = text, AutoSize = true, TabStop = false, Margin = new Padding(3) };
        button.Click += (_, _) =>
        {
            action();
            board.Focus();
        };
        return button;
    }
}
