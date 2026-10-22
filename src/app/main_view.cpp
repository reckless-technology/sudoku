#include "app/main_view.h"

#include "app/theme.h"

#include "LookMetrics.h"

#include <algorithm>
#include <utility>

namespace sudoku::app
{

MainView::MainView(const reckless::app::AppResources &resources) : resources_(resources)
{
    SetOpaque(true);
    AddChild(board_);
    AddChild(panel_);
}

void MainView::SetMenuBarView(std::unique_ptr<reckless::app::MenuBarView> bar)
{
    if (menu_bar_)
    {
        RemoveChild(*menu_bar_);
    }
    menu_bar_ = std::move(bar);
    if (menu_bar_)
    {
        AddChild(*menu_bar_);
    }
    Resized();
}

void MainView::SetOverlay(reckless::view::View *overlay)
{
    if (overlay_ != nullptr && overlay_ != overlay)
    {
        RemoveChild(*overlay_);
    }
    overlay_ = overlay;
    if (overlay_ != nullptr)
    {
        AddChild(*overlay_);
        overlay_->SetBounds(LocalBounds());
    }
    Repaint();
}

void MainView::Show(const Controller &controller)
{
    board_.SetPicture(controller.Picture());
    panel_.Show(controller);
}

reckless::gfx::RectI MainView::HeaderArea() const noexcept
{
    const int top = menu_bar_ ? menu_bar_->PreferredHeight() : 0;
    return {0, top, width(), reckless::look::kHeaderHeight};
}

void MainView::Paint(reckless::gfx::Canvas &c)
{
    c.FillAll(theme::kWindow);
    // The board's side a little lighter round the board, as a lamp over a table.
    const auto  area  = board_.bounds().ToFloat();
    const float reach = std::max(area.width(), area.height()) * 0.62f;
    c.SetGradient(reckless::gfx::Gradient{theme::kWindow.Brighter(0.06f),
                                          {area.CentreX(), area.CentreY()},
                                          theme::kWindow.Darker(0.25f),
                                          {area.CentreX() + reach, area.CentreY()},
                                          true});
    c.FillRect(area);
    const reckless::gfx::RectI header = HeaderArea();
    c.SetColor(theme::kPanel);
    c.FillRect(header);
    reckless::look::DrawHeader(c, resources_.mark(), header, "RECKLESS SUDOKU", "v" + resources_.version(), theme::kInk,
                               theme::kRule);
}

void MainView::Resized()
{
    if (overlay_ != nullptr)
    {
        overlay_->SetBounds(LocalBounds());
    }
    if (menu_bar_)
    {
        menu_bar_->SetBounds({0, 0, width(), menu_bar_->PreferredHeight()});
    }
    const int top         = HeaderArea().bottom();
    const int panel_width = SidePanel::kWidth;
    board_.SetBounds({0, top, std::max(width() - panel_width, 0), std::max(height() - top, 0)});
    panel_.SetBounds({width() - panel_width, top, panel_width, std::max(height() - top, 0)});
}

} // namespace sudoku::app
