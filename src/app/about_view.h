#pragma once
/// @file about_view.h Help > About Reckless Sudoku: a card over the window with the app's icon, its name and version,
/// the copyright, the website as a link to the person's browser, the terms it is used on, and where the third-party
/// notices are - as every Reckless app's About says them.

#include "Button.h"
#include "Canvas.h"
#include "View.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace sudoku::app
{

/** A line of text in the accent that does something when it is clicked: the card's website. */
class LinkLabel final : public reckless::view::View
{
  public:
    /** @param text The text. */
    explicit LinkLabel(std::string text);

    std::function<void()> on_click; ///< Clicked.

    /** @return The text. */
    const std::string &text() const noexcept
    {
        return text_;
    }

    /** @return Whether the pointer is over it, so it is drawn underlined. */
    bool is_over() const noexcept
    {
        return is_over_;
    }

    void Paint(reckless::gfx::Canvas &c) override;
    void MouseEnter(const reckless::view::MouseEvent &e) override;
    void MouseExit(const reckless::view::MouseEvent &e) override;
    void MouseUp(const reckless::view::MouseEvent &e) override;

  private:
    std::string text_;           ///< The text.
    bool        is_over_{false}; ///< The pointer is over it.
};

/**
 * The About card over a dimmed window: the app's icon beside its name, version and copyright and the website's link;
 * under them the terms it is used on and where the third-party notices are. A click outside the card, Escape, Return or
 * "Close" closes it (`on_close`).
 */
class AboutView final : public reckless::view::View
{
  public:
    static constexpr int kCardWidth  = 640; ///< The card across, in design pixels.
    static constexpr int kCardHeight = 310; ///< Down.

    static constexpr const char *kWebsite = "https://recklesstechnology.com"; ///< Where the link goes.

    /** @param version The program's version. */
    explicit AboutView(std::string version);

    std::function<void()>                       on_close; ///< The card closed.
    std::function<void(const std::string &url)> open_url; ///< The website's link clicked: open it in the browser.

    /** @return The card's lines of text, top to bottom: what it says, for the suites to read. */
    std::vector<std::string> Lines() const;

    /** @return The website's link. */
    LinkLabel &link() noexcept
    {
        return link_;
    }

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

    std::string                             version_; ///< The version.
    reckless::gfx::Image                    icon_;    ///< The app's icon, from the embedded icon_256.png.
    LinkLabel                               link_;    ///< The website.
    std::unique_ptr<reckless::view::Button> close_;   ///< "Close".
};

} // namespace sudoku::app
