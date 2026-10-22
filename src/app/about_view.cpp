#include "app/about_view.h"

#include "app/theme.h"

#include "Embedded.h"
#include "SkiaCanvas.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace sudoku::app
{

namespace
{

using reckless::gfx::Justify;
using reckless::gfx::RectF;
using reckless::gfx::RectI;

constexpr int kPadding      = 28;  ///< Inside the card.
constexpr int kMarkSide     = 112; ///< The icon's square.
constexpr int kTextLeft     = 136; ///< The text beside the icon, from the content's left.
constexpr int kLine         = 24;  ///< A line of body text.
constexpr int kNameHeight   = 38;  ///< The app's name.
constexpr int kCopyrightTop = 68;  ///< The copyright's line, from the icon's top: under the name and the version.
constexpr int kTermsGap     = 22;  ///< Between the icon and the terms under it.
constexpr int kSectionGap   = 16;  ///< Above the notices' line.

constexpr const char *kName      = "Reckless Sudoku";
constexpr const char *kCopyright = "Copyright 2008-2026 Jonny Reckless. Reckless Technology.";
constexpr const char *kTerms     = "Free, proprietary software, used under its end-user licence (EULA.txt).";
constexpr const char *kNotices   = "Third-party notices are in THIRD-PARTY-NOTICES.txt, installed beside the app.";

/** @return The link's font: the body face at the body text's size. */
reckless::gfx::Font LinkFont()
{
    return fonts::Small().WithHeight(17.0f);
}

} // namespace

LinkLabel::LinkLabel(std::string text) : text_(std::move(text))
{
    cursor = reckless::view::Cursor::kPointingHand;
    SetOpaque(false);
}

void LinkLabel::Paint(reckless::gfx::Canvas &c)
{
    c.SetFont(LinkFont());
    c.SetColor(theme::kAmber);
    c.DrawText(text_, LocalBounds().ToFloat(), Justify::kCentredLeft, true);
    if (is_over_)
    {
        const float underline = std::min(c.TextWidth(LinkFont(), text_), static_cast<float>(width()));
        const float y         = static_cast<float>(height()) - 4.0f;
        c.DrawLine({0.0f, y}, {underline, y}, 1.0f);
    }
}

void LinkLabel::MouseEnter(const reckless::view::MouseEvent &e)
{
    static_cast<void>(e);
    is_over_ = true;
    Repaint();
}

void LinkLabel::MouseExit(const reckless::view::MouseEvent &e)
{
    static_cast<void>(e);
    is_over_ = false;
    Repaint();
}

void LinkLabel::MouseUp(const reckless::view::MouseEvent &e)
{
    if (LocalBounds().ToFloat().Contains(e.position) && on_click)
    {
        on_click();
    }
}

AboutView::AboutView(std::string version) : version_(std::move(version)), link_("recklesstechnology.com")
{
    SetOpaque(false);
    if (const auto bytes = reckless::embedded::Find("icon_256.png"); !bytes.empty())
    {
        icon_ = reckless::gfx::DecodeImage(bytes);
    }
    link_.on_click = [this] {
        if (open_url)
        {
            open_url(kWebsite);
        }
    };
    AddChild(link_);
    close_ = std::make_unique<reckless::view::Button>("Close", theme::PlatePainter(), [this] { Close(); });
    AddChild(*close_);
}

std::vector<std::string> AboutView::Lines() const
{
    return {kName, "Version " + version_, kCopyright, link_.text(), kTerms, kNotices};
}

RectI AboutView::CardArea() const noexcept
{
    const int card_width  = std::min(kCardWidth, width() - 40);
    const int card_height = std::min(kCardHeight, height() - 40);
    return RectI{(width() - card_width) / 2, (height() - card_height) / 2, card_width, card_height};
}

void AboutView::Paint(reckless::gfx::Canvas &c)
{
    c.FillAll(theme::kWindow.Darker(0.6f).WithAlpha(0.7f));
    const RectF card = CardArea().ToFloat();
    c.SetColor(theme::kPanel);
    c.FillRoundedRect(card, 12.0f);
    c.SetColor(theme::kRule);
    c.DrawRoundedRect(card.Reduced(0.5f), 12.0f, 1.0f);

    const float left      = card.x() + kPadding;
    const float right     = card.right() - kPadding;
    const float text_left = left + kTextLeft;
    float       y         = card.y() + kPadding;
    if (icon_.IsValid())
    {
        c.SetResampling(reckless::gfx::Resampling::kHigh);
        c.DrawImage(icon_, RectF{left, y, static_cast<float>(kMarkSide), static_cast<float>(kMarkSide)});
    }
    c.SetFont(fonts::Body().WithHeight(28.0f).Bold());
    c.SetColor(theme::kInk);
    c.DrawText(kName, RectF{text_left, y, right - text_left, static_cast<float>(kNameHeight)}, Justify::kCentredLeft,
               true);
    c.SetFont(fonts::Small());
    c.SetColor(theme::kMuted);
    c.DrawText("Version " + version_, RectF{text_left, y + kNameHeight, right - text_left, 22.0f},
               Justify::kCentredLeft, true);
    c.SetFont(fonts::Small().WithHeight(17.0f));
    c.SetColor(theme::kInk);
    c.DrawText(kCopyright, RectF{text_left, y + kCopyrightTop, right - text_left, static_cast<float>(kLine)},
               Justify::kCentredLeft, true);
    // The link, a line under the copyright, is a view of its own.

    y += kMarkSide + kTermsGap;
    c.SetColor(theme::kInk);
    c.DrawText(kTerms, RectF{left, y, right - left, static_cast<float>(kLine)}, Justify::kCentredLeft, true);
    y += kLine + kSectionGap;
    c.SetColor(theme::kRule);
    c.DrawHorizontalLine(static_cast<int>(y) - 6, left, right);
    c.SetFont(fonts::Small());
    c.SetColor(theme::kMuted);
    // On one line, squeezed a little where a face draws it wider than the card.
    c.DrawFittedText(kNotices,
                     RectI{static_cast<int>(left), static_cast<int>(y), static_cast<int>(right - left), kLine},
                     Justify::kCentredLeft, 1, 0.85f);
}

void AboutView::Resized()
{
    const RectI card      = CardArea();
    const int   text_left = card.x() + kPadding + kTextLeft;
    const int   link_y    = card.y() + kPadding + kCopyrightTop + kLine; // the line under the copyright
    const int   width     = static_cast<int>(std::ceil(reckless::gfx::MeasureText(LinkFont(), link_.text()))) + 4;
    link_.SetBounds({text_left, link_y, std::min(width, card.right() - kPadding - text_left), kLine});
    close_->SetBounds({card.right() - kPadding - 120, card.bottom() - 22 - 38, 120, 38});
}

void AboutView::MouseDown(const reckless::view::MouseEvent &e)
{
    if (!CardArea().ToFloat().Contains(e.position))
    {
        Close();
    }
}

bool AboutView::WantsKeyboardFocus() const
{
    return true;
}

bool AboutView::KeyPressed(const reckless::view::KeyEvent &key)
{
    if (key.key == reckless::view::Key::kEscape || key.key == reckless::view::Key::kReturn)
    {
        Close();
        return true;
    }
    return false;
}

void AboutView::Close()
{
    if (on_close)
    {
        const auto closed = on_close;
        closed();
    }
}

} // namespace sudoku::app
