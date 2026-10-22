#pragma once
/// @file about_view.h Help > About Reckless Sudoku: a card over the window with the app's icon, name, version and
/// copyright, its licence and the notices of the third-party parts the program carries, read from the files it embeds.

#include "Button.h"
#include "TextView.h"
#include "View.h"

#include <functional>
#include <memory>
#include <string>

namespace sudoku::app
{

/**
 * The About card over a dimmed window: the app's icon, the name, the version, the copyright, and the licence and the
 * third-party notices in a scrolling text. A click outside the card, Escape, or "Close" closes it (`on_close`).
 */
class AboutView final : public reckless::view::View
{
  public:
    static constexpr int kCardWidth  = 620; ///< The card across, in design pixels.
    static constexpr int kCardHeight = 560; ///< Down.

    /**
     * @param version The program's version.
     * @param licence The licence's text; the embedded `licence.txt` where empty.
     * @param notices The third-party notices; the embedded `third_party_notices.txt` where empty.
     */
    AboutView(std::string version, std::string licence = {}, std::string notices = {});

    std::function<void()> on_close; ///< The card closed.

    /** @return The text shown in the scrolling view. */
    const std::string &text() const noexcept;

    /** @return The card's place in the view. */
    reckless::gfx::RectI CardArea() const noexcept;

    void Paint(reckless::gfx::Canvas &c) override;
    void Resized() override;
    void MouseDown(const reckless::view::MouseEvent &e) override;
    bool WantsKeyboardFocus() const override;
    bool KeyPressed(const reckless::view::KeyEvent &key) override;

  private:
    /** The card closed, told. */
    void Close();

    std::string                               version_; ///< The version.
    reckless::gfx::Image                      icon_;    ///< The app's icon, from the embedded icon_256.png.
    std::unique_ptr<reckless::view::TextView> text_;    ///< The licence and the notices.
    std::unique_ptr<reckless::view::Button>   close_;   ///< "Close".
};

} // namespace sudoku::app
