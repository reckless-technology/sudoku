#include "app/board_view.h"

#include "app/theme.h"
#include "solver/digits.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

namespace sudoku::app
{

namespace
{

using reckless::gfx::PointF;
using reckless::gfx::RectF;

constexpr float kMargin =
    36.0f;                 ///< Round the board, in design pixels: room for the columns' letters and the rows' numbers.
constexpr int kClear = 10; ///< The digit menu's Clear: its id.

/**
 * @param board The board's square.
 * @param cell A square.
 * @return The square's area.
 */
RectF CellArea(RectF board, int cell)
{
    const float side = board.width() / 9.0f;
    return RectF{board.x() + static_cast<float>(geometry::Column(cell)) * side,
                 board.y() + static_cast<float>(geometry::Row(cell)) * side, side, side};
}

/**
 * @param cells Squares.
 * @param cell A square.
 * @return Whether it is among them.
 */
bool Holds(const std::vector<int> &cells, int cell)
{
    return std::find(cells.begin(), cells.end(), cell) != cells.end();
}

} // namespace

BoardView::BoardView()
{
    SetOpaque(false);
}

void BoardView::SetPicture(BoardPicture picture)
{
    if (!(picture == picture_))
    {
        picture_ = std::move(picture);
        Repaint();
    }
}

RectF BoardView::BoardArea() const noexcept
{
    const float side =
        std::max(0.0f, std::min(static_cast<float>(width()), static_cast<float>(height())) - 2 * kMargin);
    return RectF{(static_cast<float>(width()) - side) / 2.0f, (static_cast<float>(height()) - side) / 2.0f, side, side};
}

int BoardView::CellAt(PointF point) const noexcept
{
    const RectF board = BoardArea();
    if (board.width() <= 0.0f || point.x < board.x() || point.y < board.y() || point.x >= board.right() ||
        point.y >= board.bottom())
    {
        return -1;
    }
    const float side   = board.width() / 9.0f;
    const int   row    = std::min(8, static_cast<int>((point.y - board.y()) / side));
    const int   column = std::min(8, static_cast<int>((point.x - board.x()) / side));
    return geometry::Cell(row, column);
}

void BoardView::PaintBoard(reckless::gfx::Canvas &c, RectF board, const BoardPicture &picture, bool has_focus)
{
    if (board.width() < 18.0f)
    {
        return;
    }
    const float side = board.width() / 9.0f;
    c.SetColor(theme::kBoard);
    c.FillRoundedRect(board.Expanded(4.0f), 6.0f);

    // The squares' washes: a clash red, the chosen square amber, a placed value green, the step's squares faint amber.
    for (int cell = 0; cell < geometry::kCellCount; ++cell)
    {
        const RectF area = CellArea(board, cell);
        if (Holds(picture.clashes, cell))
        {
            c.SetColor(theme::kBad.WithAlpha(0.32f));
        }
        else if (Holds(picture.placed, cell))
        {
            c.SetColor(theme::kGood.WithAlpha(0.24f));
        }
        else if (Holds(picture.pattern, cell))
        {
            c.SetColor(theme::kAmber.WithAlpha(0.12f));
        }
        else
        {
            continue;
        }
        c.FillRect(area);
    }
    const RectF chosen = CellArea(board, picture.selected);
    c.SetColor(theme::kAmber.WithAlpha(has_focus ? 0.22f : 0.10f));
    c.FillRect(chosen);

    // The values, those the lit step read ringed; and the empty squares' pencil marks, those the lit step read on an
    // amber disc, those it struck red.
    const reckless::gfx::Font given  = reckless::gfx::Font(side * 0.58f).Bold();
    const reckless::gfx::Font solved = reckless::gfx::Font(side * 0.58f);
    const reckless::gfx::Font marks  = fonts::Mono(side * 0.24f);
    const float               third  = side / 3.0f;
    for (int cell = 0; cell < geometry::kCellCount; ++cell)
    {
        const auto  index = static_cast<std::size_t>(cell);
        const RectF area  = CellArea(board, cell);
        if (const int value = picture.values[index]; value != 0)
        {
            if (picture.IsReasonValue(cell))
            {
                c.SetColor(theme::kAmber);
                c.DrawEllipse(area.Reduced(side * 0.12f), std::max(1.5f, side * 0.04f));
            }
            c.SetFont(picture.is_given[index] ? given : solved);
            c.SetColor(picture.is_given[index] ? theme::kInk : theme::kAmber);
            c.DrawText(std::to_string(value), area, reckless::gfx::Justify::kCentred, false);
            continue;
        }
        c.SetFont(marks);
        for (int digit = 1; digit <= 9; ++digit)
        {
            const MarkLook look = picture.LookOf(cell, digit);
            if (look == MarkLook::kHidden)
            {
                continue;
            }
            const RectF mark{area.x() + static_cast<float>((digit - 1) % 3) * third,
                             area.y() + static_cast<float>((digit - 1) / 3) * third, third, third};
            if (look == MarkLook::kReason)
            {
                c.SetColor(theme::kAmber.WithAlpha(0.30f));
                c.FillEllipse(mark.Reduced(third * 0.06f));
            }
            c.SetColor(look == MarkLook::kStruck   ? theme::kBad
                       : look == MarkLook::kReason ? theme::kAmber
                                                   : theme::kMuted);
            c.DrawText(std::to_string(digit), mark, reckless::gfx::Justify::kCentred, false);
        }
    }

    // The lines: thin between the squares, thick round the boxes and the board.
    for (int line = 1; line < 9; ++line)
    {
        if (line % 3 == 0)
        {
            continue;
        }
        const float offset = static_cast<float>(line) * side;
        c.SetColor(theme::kThinLine);
        c.DrawLine({board.x() + offset, board.y()}, {board.x() + offset, board.bottom()}, 1.0f);
        c.DrawLine({board.x(), board.y() + offset}, {board.right(), board.y() + offset}, 1.0f);
    }
    c.SetColor(theme::kThickLine);
    for (int line = 3; line < 9; line += 3)
    {
        const float offset = static_cast<float>(line) * side;
        c.DrawLine({board.x() + offset, board.y()}, {board.x() + offset, board.bottom()}, 2.5f);
        c.DrawLine({board.x(), board.y() + offset}, {board.right(), board.y() + offset}, 2.5f);
    }
    c.DrawRoundedRect(board, 3.0f, 2.5f);

    // The squares' names, as a chess board's: the columns' letters under the board, the rows' numbers at its left.
    const float band = std::min(kMargin - 6.0f, side * 0.5f);
    c.SetFont(fonts::Mono(std::min(17.0f, side * 0.28f)));
    c.SetColor(theme::kMuted);
    for (int line = 0; line < 9; ++line)
    {
        const float offset = static_cast<float>(line) * side;
        c.DrawText(std::string(1, geometry::ColumnLetter(line)),
                   RectF{board.x() + offset, board.bottom() + 6.0f, side, band}, reckless::gfx::Justify::kCentred,
                   false);
        c.DrawText(std::to_string(geometry::RowNumber(line)),
                   RectF{board.x() - 6.0f - band, board.y() + offset, band, side}, reckless::gfx::Justify::kCentred,
                   false);
    }
    if (has_focus)
    {
        c.SetColor(theme::kAmber);
        c.DrawRect(chosen.Reduced(1.5f), 2.0f);
    }
}

void BoardView::Paint(reckless::gfx::Canvas &c)
{
    PaintBoard(c, BoardArea(), picture_, has_focus_);
}

void BoardView::MouseDown(const reckless::view::MouseEvent &e)
{
    GrabKeyboardFocus();
    const int cell = CellAt(e.position);
    if (cell < 0)
    {
        return;
    }
    if (on_select)
    {
        on_select(cell);
    }
    if (e.right)
    {
        OfferDigits(e.position);
    }
}

bool BoardView::WantsKeyboardFocus() const
{
    return true;
}

bool BoardView::KeepsKeyboardFocus() const
{
    return true;
}

bool BoardView::KeyPressed(const reckless::view::KeyEvent &key)
{
    using reckless::view::Key;
    if (key.modifiers.command || key.modifiers.alt)
    {
        return false;
    }
    const int  cell = picture_.selected;
    const auto move = [this, cell](int rows, int columns) {
        const int row    = std::clamp(geometry::Row(cell) + rows, 0, 8);
        const int column = std::clamp(geometry::Column(cell) + columns, 0, 8);
        if (on_select)
        {
            on_select(geometry::Cell(row, column));
        }
        return true;
    };
    const auto enter = [this, cell](int digit) {
        if (on_digit)
        {
            on_digit(cell, digit);
        }
        return true;
    };
    switch (key.key)
    {
    case Key::kUp:
        return move(-1, 0);
    case Key::kDown:
        return move(1, 0);
    case Key::kLeft:
        return move(0, -1);
    case Key::kRight:
        return move(0, 1);
    case Key::kBackspace:
    case Key::kDelete:
        return enter(0);
    case Key::kCharacter:
        if (key.character >= U'1' && key.character <= U'9')
        {
            return enter(static_cast<int>(key.character - U'0'));
        }
        if (key.character == U'0' || key.character == U' ')
        {
            return enter(0);
        }
        return false;
    default:
        return false;
    }
}

void BoardView::FocusGained()
{
    has_focus_ = true;
    Repaint();
}

void BoardView::FocusLost()
{
    has_focus_ = false;
    Repaint();
}

void BoardView::OfferDigits(PointF at)
{
    std::vector<reckless::view::MenuItem> items;
    for (int digit = 1; digit <= 9; ++digit)
    {
        reckless::view::MenuItem item;
        item.text = std::to_string(digit);
        item.id   = digit;
        items.push_back(std::move(item));
    }
    reckless::view::MenuItem separator;
    separator.separator = true;
    items.push_back(separator);
    reckless::view::MenuItem clear;
    clear.text = "Clear";
    clear.id   = kClear;
    items.push_back(std::move(clear));
    const int cell = picture_.selected;
    menu_ = reckless::view::MenuSession::Open(*this, std::move(items), theme::MenuLook(),
                                              {static_cast<int>(at.x), static_cast<int>(at.y)}, [this, cell](int id) {
                                                  menu_.reset();
                                                  if (id > 0 && on_digit)
                                                  {
                                                      on_digit(cell, id == kClear ? 0 : id);
                                                  }
                                              });
}

} // namespace sudoku::app
