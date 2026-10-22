#pragma once
/// @file theme.h The window's colours and fonts: the Reckless apps' dark look - the cream ink, the amber accent, Noto
/// Serif for the interface and Monaspace Xenon for the pencil marks and the squares' names - and the board's.

#include "AppTheme.h"
#include "Button.h"
#include "Color.h"
#include "Embedded.h"
#include "ListView.h"
#include "LookDrawing.h"
#include "LookPalette.h"
#include "MenuBarView.h"
#include "PanelStyle.h"
#include "SkiaCanvas.h"
#include "Text.h"

#include <string_view>

namespace sudoku::app::theme
{

using reckless::gfx::Color;

inline constexpr Color kWindow{0xff29282cu};   ///< The window's colour, behind the board.
inline constexpr Color kPanel{0xff1b1a1du};    ///< The side panel and the header: one step darker.
inline constexpr Color kRow{0xff222125u};      ///< A row of a list.
inline constexpr Color kRule{0xff3b3936u};     ///< The rules and the frames round a section.
inline constexpr Color kInk{0xffe8dec8u};      ///< The text, and the givens.
inline constexpr Color kMuted{0xff9c9483u};    ///< Secondary text, and the pencil marks.
inline constexpr Color kFaint{0xff5f5a52u};    ///< A hint, a disabled item.
inline constexpr Color kAmber{0xffe2a23eu};    ///< The accent: a lit plate, the solver's values.
inline constexpr Color kAmberDim{0xff6e5428u}; ///< The accent at rest: a plate's edge.
inline constexpr Color kSelect{0xff5d4423u};   ///< The menu under the pointer, a list's chosen row.
inline constexpr Color kGood{0xff86c9a2u};     ///< A solved board, a placed value's square.
inline constexpr Color kBad{0xffe2705fu};      ///< A clash, a struck candidate.
inline constexpr Color kPlate{0xff232125u};    ///< A button's plate.

inline constexpr Color kBoard{0xff1f1e22u};     ///< The board's squares.
inline constexpr Color kThinLine{0xff47443fu};  ///< The lines between squares.
inline constexpr Color kThickLine{0xffb3a88fu}; ///< The lines round the boxes and the board.

/** @return The window's colours as the toolkit's app shell and its widgets take them. */
inline reckless::app::Theme Shell()
{
    return {kWindow, kRule, kPanel, kRow, kInk, kMuted, kFaint, kAmber, kAmberDim, kGood, kBad};
}

/** @return The series' palette for the buttons: the plate a step lighter than the panel, its edge the accent at rest.
 */
inline reckless::look::Palette Buttons()
{
    return {kWindow, kInk, kAmber, kPlate, kAmberDim};
}

} // namespace sudoku::app::theme

namespace sudoku::app::fonts
{

using reckless::gfx::Font;

/** Hands the two families' faces to Skia from the files the binary carries; once, before anything draws text. */
inline void InstallFaces()
{
    reckless::gfx::SetSerifFaces(reckless::embedded::Find("NotoSerif-Regular.ttf"),
                                 reckless::embedded::Find("NotoSerif-Bold.ttf"));
    reckless::gfx::SetMonospaceFaces(reckless::embedded::Find("MonaspaceXenon-Regular.otf"),
                                     reckless::embedded::Find("MonaspaceXenon-Bold.otf"));
}

/** @return The interface's text. */
inline Font Body()
{
    return Font(19.0f);
}

/** @return Smaller text: a caption, a step in the list. */
inline Font Small()
{
    return Font(16.0f);
}

/** @return A plate's caption: the body face, bold, a little smaller. */
inline Font Plate()
{
    return Font(16.5f).Bold();
}

/** @return Where the puzzle stands, large: "Solved", "38 of 81". */
inline Font Headline()
{
    return Font(30.0f).Bold();
}

/**
 * @param height The text's height, in design pixels.
 * @return The monospaced family: the pencil marks.
 */
inline Font Mono(float height)
{
    return Font(height).WithFamily(reckless::gfx::FontFamily::kMonospace);
}

} // namespace sudoku::app::fonts

namespace sudoku::app::theme
{

/**
 * The window's buttons: the series' button plate in the window's colours (look::DrawButtonPlate), with a toggle's lamp
 * at its left (look::DrawPlateLamp), and its caption in the interface's face.
 * @return The painter.
 */
inline reckless::view::Button::Painter PlatePainter()
{
    return [](reckless::gfx::Canvas &c, reckless::gfx::RectI bounds, std::string_view text,
              reckless::view::Button::Look look) {
        reckless::look::DrawButtonPlate(c, Buttons(), bounds, look.on && !look.lamp, look.over && !look.dimmed,
                                        look.held);
        if (look.lamp)
        {
            reckless::look::DrawPlateLamp(c, bounds, look.on);
        }
        c.SetFont(fonts::Plate());
        c.SetColor(look.dimmed ? kInk.WithAlpha(0.35f) : kInk);
        const float left = look.lamp ? 18.0f : 4.0f;
        c.DrawText(text,
                   reckless::gfx::RectF{left, 0.0f, static_cast<float>(bounds.width()) - left - 4.0f,
                                        static_cast<float>(bounds.height())},
                   reckless::gfx::Justify::kCentred, true);
    };
}

/** @return The steps list's look: the panel's colours, the chosen step on the selection's amber. */
inline reckless::view::ListView::Style StepsLook()
{
    reckless::view::ListView::Style style;
    style.font            = fonts::Small().WithHeight(15.5f);
    style.row_height      = 26;
    style.header_height   = 0;
    style.padding         = 8;
    style.background      = kWindow.Darker(0.3f);
    style.stripe          = kRow.WithAlpha(0.5f);
    style.text            = kInk;
    style.selection       = kSelect;
    style.selected_text   = kInk.Brighter(0.1f);
    style.grid            = Color{0x00000000u};
    style.focused_outline = Color{0x00000000u};
    return style;
}

/** @return The menus' look: the panel's colours under the interface's face. */
inline reckless::view::MenuStyle MenuLook()
{
    return reckless::panel::MenuStyle(Shell().Palette());
}

/**
 * The drawn menu bar (Linux's, and the window drawn to an image): the panel's colour under the interface's face.
 * @return The bar's look.
 */
inline reckless::app::MenuBarStyle MenuBarLook()
{
    reckless::app::MenuBarStyle style;
    style.background    = kPanel;
    style.rule          = kRule;
    style.text          = kInk;
    style.disabled_text = kFaint;
    style.highlight     = kSelect;
    style.font          = fonts::Body().WithHeight(18.5f);
    style.height        = 34;
    style.padding       = 11;
    style.menu          = MenuLook();
    return style;
}

} // namespace sudoku::app::theme
