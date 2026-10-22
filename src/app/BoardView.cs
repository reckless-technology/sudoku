using System.ComponentModel;
using System.Drawing.Drawing2D;
using System.Drawing.Text;

namespace Sudoku;

/// <summary>
/// The board, drawn square at whatever size the window gives it: the givens in bold, the solver's values in blue, the
/// pencil marks small, and a step lit - the squares it reasoned from, the value it placed, the candidates it struck in
/// red. A square is chosen with the mouse or the arrow keys; a digit key enters a given there, and 0, Delete,
/// Backspace or Space clears it.
/// </summary>
internal sealed class BoardView : Control
{
    private static readonly Color GivenInk = Color.FromArgb(20, 20, 20);
    private static readonly Color SolvedInk = Color.FromArgb(30, 90, 200);
    private static readonly Color MarkInk = Color.FromArgb(120, 120, 120);
    private static readonly Color StruckInk = Color.FromArgb(210, 40, 40);
    private static readonly Color SelectedFill = Color.FromArgb(255, 236, 160);
    private static readonly Color UnfocusedFill = Color.FromArgb(240, 236, 220);
    private static readonly Color PatternFill = Color.FromArgb(214, 228, 250);
    private static readonly Color PlacedFill = Color.FromArgb(206, 240, 206);
    private static readonly Color ClashFill = Color.FromArgb(250, 200, 200);

    private Game? game;
    private Step? highlight;
    private int selected = Geometry.Cell(4, 4);
    private bool showsPencilMarks = true;

    public BoardView()
    {
        SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.OptimizedDoubleBuffer | ControlStyles.UserPaint |
                     ControlStyles.ResizeRedraw | ControlStyles.Selectable,
                 true);
        TabStop = true;
        BackColor = Color.White;
    }

    /// <summary>A digit entered in a square: the square, and the digit (0 to clear it).</summary>
    public event Action<int, int>? DigitEntered;

    /// <summary>The game shown.</summary>
    [Browsable(false)]
    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
    public Game? Game
    {
        get => game;
        set
        {
            game = value;
            Invalidate();
        }
    }

    /// <summary>The step lit; null for none.</summary>
    [Browsable(false)]
    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
    public Step? Highlight
    {
        get => highlight;
        set
        {
            highlight = value;
            Invalidate();
        }
    }

    /// <summary>The square chosen, which a digit key fills.</summary>
    [Browsable(false)]
    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
    public int Selected
    {
        get => selected;
        set
        {
            selected = Math.Clamp(value, 0, Geometry.CellCount - 1);
            Invalidate();
        }
    }

    /// <summary>Whether an empty square shows the digits it may still take.</summary>
    [DefaultValue(true)]
    public bool ShowsPencilMarks
    {
        get => showsPencilMarks;
        set
        {
            showsPencilMarks = value;
            Invalidate();
        }
    }

    /// <param name="point">A point in the control.</param>
    /// <returns>The square under it; -1 for none.</returns>
    public int CellAt(Point point)
    {
        RectangleF board = BoardBounds();
        if (board.Width <= 0 || !board.Contains(point))
        {
            return -1;
        }
        float size = board.Width / 9;
        int row = Math.Min(8, (int)((point.Y - board.Y) / size));
        int column = Math.Min(8, (int)((point.X - board.X) / size));
        return Geometry.Cell(row, column);
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        Graphics graphics = e.Graphics;
        graphics.Clear(BackColor);
        RectangleF board = BoardBounds();
        if (game is null || board.Width < 18)
        {
            return;
        }
        graphics.SmoothingMode = SmoothingMode.AntiAlias;
        graphics.TextRenderingHint = TextRenderingHint.AntiAliasGridFit;
        float size = board.Width / 9;
        PaintFills(graphics, board, size);
        PaintValues(graphics, board, size);
        PaintLines(graphics, board, size);
    }

    protected override bool IsInputKey(Keys keyData) =>
        (keyData & Keys.KeyCode) is Keys.Up or Keys.Down or Keys.Left or Keys.Right || base.IsInputKey(keyData);

    protected override void OnKeyDown(KeyEventArgs e)
    {
        base.OnKeyDown(e);
        if (e.Control || e.Alt)
        {
            return;
        }
        int row = Geometry.Row(selected);
        int column = Geometry.Column(selected);
        int? digit = null;
        switch (e.KeyCode)
        {
            case Keys.Up:
                row = Math.Max(0, row - 1);
                break;
            case Keys.Down:
                row = Math.Min(8, row + 1);
                break;
            case Keys.Left:
                column = Math.Max(0, column - 1);
                break;
            case Keys.Right:
                column = Math.Min(8, column + 1);
                break;
            case >= Keys.D1 and <= Keys.D9:
                digit = e.KeyCode - Keys.D0;
                break;
            case >= Keys.NumPad1 and <= Keys.NumPad9:
                digit = e.KeyCode - Keys.NumPad0;
                break;
            case Keys.D0 or Keys.NumPad0 or Keys.Delete or Keys.Back or Keys.Space:
                digit = 0;
                break;
            default:
                return;
        }
        e.Handled = true;
        e.SuppressKeyPress = true;
        if (digit is { } entered)
        {
            DigitEntered?.Invoke(selected, entered);
        }
        else
        {
            Selected = Geometry.Cell(row, column);
        }
    }

    protected override void OnMouseDown(MouseEventArgs e)
    {
        Focus();
        if (CellAt(e.Location) is var cell and >= 0)
        {
            Selected = cell;
        }
        base.OnMouseDown(e);
    }

    protected override void OnGotFocus(EventArgs e)
    {
        base.OnGotFocus(e);
        Invalidate();
    }

    protected override void OnLostFocus(EventArgs e)
    {
        base.OnLostFocus(e);
        Invalidate();
    }

    /// <returns>The board's square, centred, with a margin; in pixels.</returns>
    private RectangleF BoardBounds()
    {
        float margin = 10f * DeviceDpi / 96f;
        float size = Math.Max(0, Math.Min(ClientSize.Width, ClientSize.Height) - 2 * margin);
        return new RectangleF((ClientSize.Width - size) / 2, (ClientSize.Height - size) / 2, size, size);
    }

    private static RectangleF CellBounds(RectangleF board, float size, int cell) =>
        new(board.X + Geometry.Column(cell) * size, board.Y + Geometry.Row(cell) * size, size, size);

    private void PaintFills(Graphics graphics, RectangleF board, float size)
    {
        var pattern = new HashSet<int>(highlight?.Pattern ?? []);
        var placed = new HashSet<int>(highlight?.Placements.Select(p => p.Cell) ?? []);
        var clashes = new HashSet<int>(game!.Clashes);
        for (int cell = 0; cell < Geometry.CellCount; cell++)
        {
            Color? fill = clashes.Contains(cell) ? ClashFill
                          : cell == selected     ? (Focused ? SelectedFill : UnfocusedFill)
                          : placed.Contains(cell) ? PlacedFill
                          : pattern.Contains(cell) ? PatternFill
                                                   : null;
            if (fill is { } colour)
            {
                using var brush = new SolidBrush(colour);
                graphics.FillRectangle(brush, CellBounds(board, size, cell));
            }
        }
    }

    private void PaintValues(Graphics graphics, RectangleF board, float size)
    {
        string family = Font.FontFamily.Name;
        using var givenFont = new Font(family, size * 0.56f, FontStyle.Bold, GraphicsUnit.Pixel);
        using var solvedFont = new Font(family, size * 0.56f, FontStyle.Regular, GraphicsUnit.Pixel);
        using var markFont = new Font(family, size * 0.25f, FontStyle.Regular, GraphicsUnit.Pixel);
        using var centred = new StringFormat { Alignment = StringAlignment.Center, LineAlignment = StringAlignment.Center };
        using var givenBrush = new SolidBrush(GivenInk);
        using var solvedBrush = new SolidBrush(SolvedInk);
        using var markBrush = new SolidBrush(MarkInk);
        using var struckBrush = new SolidBrush(StruckInk);
        var struck = (highlight?.Eliminations ?? []).ToLookup(c => c.Cell, c => c.Digit);
        for (int cell = 0; cell < Geometry.CellCount; cell++)
        {
            RectangleF bounds = CellBounds(board, size, cell);
            int value = game!.Value(cell);
            if (value != 0)
            {
                bool isGiven = game.IsGiven(cell);
                graphics.DrawString(value.ToString(), isGiven ? givenFont : solvedFont, isGiven ? givenBrush : solvedBrush,
                                    bounds, centred);
                continue;
            }
            if (!showsPencilMarks)
            {
                continue;
            }
            int candidates = game.Candidates(cell);
            float third = size / 3;
            for (int digit = 1; digit <= 9; digit++)
            {
                bool isStruck = struck[cell].Contains(digit);
                if (!Digits.Contains(candidates, digit) && !isStruck)
                {
                    continue;
                }
                var mark = new RectangleF(bounds.X + (digit - 1) % 3 * third, bounds.Y + (digit - 1) / 3 * third, third, third);
                graphics.DrawString(digit.ToString(), markFont, isStruck ? struckBrush : markBrush, mark, centred);
            }
        }
    }

    private void PaintLines(Graphics graphics, RectangleF board, float size)
    {
        float scale = DeviceDpi / 96f;
        using var thin = new Pen(Color.FromArgb(170, 170, 170), 1f * scale);
        using var thick = new Pen(Color.FromArgb(40, 40, 40), 2.5f * scale);
        for (int line = 0; line <= 9; line++)
        {
            if (line % 3 == 0)
            {
                continue;
            }
            float offset = line * size;
            graphics.DrawLine(thin, board.X + offset, board.Y, board.X + offset, board.Bottom);
            graphics.DrawLine(thin, board.X, board.Y + offset, board.Right, board.Y + offset);
        }
        for (int line = 0; line <= 9; line += 3)
        {
            float offset = line * size;
            graphics.DrawLine(thick, board.X + offset, board.Y, board.X + offset, board.Bottom);
            graphics.DrawLine(thick, board.X, board.Y + offset, board.Right, board.Y + offset);
        }
    }
}
