#pragma once
/// @file board_view.h The board: drawn square at the largest size its view holds, telling what the person does over it
/// - a square chosen, a digit entered.

#include "app/board_picture.h"

#include "Canvas.h"
#include "Geometry.h"
#include "Menu.h"
#include "View.h"

#include <functional>
#include <memory>

namespace sudoku::app
{

/**
 * The board, square and centred: the givens in bold cream, the solver's values in amber, the pencil marks in the
 * monospaced family, and the lit step - its squares washed in amber, the pencil marks it read on amber discs and the
 * values that rule a single's other choices out ringed, the values it placed in green, the candidates it struck in red;
 * its marks drawn even with the pencil marks hidden. A click chooses a square and a right click offers its digits; the
 * arrows move the choice, 1 to 9 enter a given there, and 0, Delete, Backspace or Space clear it.
 */
class BoardView final : public reckless::view::View
{
  public:
    BoardView();

    std::function<void(int cell)>            on_select; ///< A square chosen, by a click or the arrows.
    std::function<void(int cell, int digit)> on_digit;  ///< A digit entered in a square; 0 to clear it.

    /**
     * What the board shows from now on; repainted when it differs.
     * @param picture The picture.
     */
    void SetPicture(BoardPicture picture);

    /** @return What it shows. */
    const BoardPicture &picture() const noexcept
    {
        return picture_;
    }

    /**
     * The board as the view draws it, onto any canvas: the render tool's.
     * @param c The canvas.
     * @param board The board's square, in the canvas's pixels.
     * @param picture What to show.
     * @param has_focus Whether the chosen square is drawn as the keyboard's.
     */
    static void PaintBoard(reckless::gfx::Canvas &c, reckless::gfx::RectF board, const BoardPicture &picture,
                           bool has_focus);

    /** @return The board's square in the view: the largest that fits, with a margin, centred. */
    reckless::gfx::RectF BoardArea() const noexcept;

    /**
     * @param point A point in the view.
     * @return The square under it; -1 for none.
     */
    int CellAt(reckless::gfx::PointF point) const noexcept;

    void Paint(reckless::gfx::Canvas &c) override;
    void MouseDown(const reckless::view::MouseEvent &e) override;
    bool WantsKeyboardFocus() const override;

    /**
     * The board keeps the keyboard against a press on a view that takes none - every plate of the panel - so the keys
     * still enter digits after the panel is used.
     * @return True.
     */
    bool KeepsKeyboardFocus() const override;

    bool KeyPressed(const reckless::view::KeyEvent &key) override;
    void FocusGained() override;
    void FocusLost() override;

  private:
    /**
     * The digit menu for a square: 1 to 9, and Clear. The square is given rather than read from the picture, which
     * is only rebuilt on the next frame and so still names the square chosen before this click.
     * @param cell The square the digits are entered in.
     * @param at Where it opens, in the view.
     */
    void OfferDigits(int cell, reckless::gfx::PointF at);

    BoardPicture                                 picture_;          ///< What it shows.
    std::unique_ptr<reckless::view::MenuSession> menu_;             ///< The digit menu while it is open.
    bool                                         has_focus_{false}; ///< Whether it has the keyboard.
};

} // namespace sudoku::app
