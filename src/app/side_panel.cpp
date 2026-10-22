#include "app/side_panel.h"

#include "app/theme.h"

#include <array>
#include <string_view>
#include <utility>

namespace sudoku::app
{

namespace
{

using reckless::gfx::RectI;

constexpr int kMargin      = 18; ///< The panel's margin.
constexpr int kPlateHeight = 36; ///< A plate's height.
constexpr int kGap         = 6;  ///< Between plates.

} // namespace

MessageView::MessageView()
{
    SetInterceptsMouse(false, false);
}

void MessageView::SetText(std::string text)
{
    if (text != text_)
    {
        text_ = std::move(text);
        Repaint();
    }
}

void MessageView::Paint(reckless::gfx::Canvas &c)
{
    c.SetFont(fonts::Small());
    c.SetColor(theme::kMuted);
    c.DrawFittedText(text_, LocalBounds(), reckless::gfx::Justify::kTopLeft, 3);
}

std::string StepsModel::CellText(std::size_t row, std::size_t column) const
{
    static_cast<void>(column);
    return row < steps_.size() ? steps_[row] : std::string();
}

SidePanel::SidePanel()
{
    SetOpaque(true);
    const reckless::app::Theme shell = theme::Shell();
    headline_                        = std::make_unique<reckless::app::TextLine>(theme::kInk, fonts::Headline());
    message_                         = std::make_unique<MessageView>();
    AddChild(*headline_);
    AddChild(*message_);
    solve_section_ = std::make_unique<reckless::app::Section>(shell, "Solve");
    board_section_ = std::make_unique<reckless::app::Section>(shell, "Board");
    steps_section_ = std::make_unique<reckless::app::Section>(shell, "Steps");
    for (auto *section : {solve_section_.get(), board_section_.get(), steps_section_.get()})
    {
        AddChild(*section);
    }
    const auto tell = [](const std::function<void()> &callback) {
        if (callback)
        {
            callback();
        }
    };
    clue_  = AddPlate("Clue", [this, tell] { tell(on_clue); });
    solve_ = AddPlate("Solve", [this, tell] { tell(on_solve); });
    check_ = AddPlate("Check", [this, tell] { tell(on_check); });
    undo_  = AddPlate("Undo", [this, tell] { tell(on_undo); });
    clear_ = AddPlate("Clear", [this, tell] { tell(on_clear); });
    // The switch tells its state after the press has flipped it.
    pencil_marks_ = AddPlate(
        "Pencil marks",
        [this] {
            if (on_pencil_marks)
            {
                on_pencil_marks(pencil_marks_->on());
            }
        },
        true);
    steps_ = std::make_unique<reckless::view::ListView>(theme::StepsLook());
    steps_->SetModel(&steps_model_);
    steps_->SetSelectionMode(reckless::view::ListView::SelectionMode::kSingle);
    steps_->on_selection_changed = [this] {
        if (is_showing_ || !on_step)
        {
            return;
        }
        on_step(steps_->current_row());
    };
    AddChild(*steps_);
}

std::unique_ptr<reckless::view::Button> SidePanel::AddPlate(std::string text, std::function<void()> clicked,
                                                            bool is_toggle)
{
    auto plate =
        std::make_unique<reckless::view::Button>(std::move(text), theme::PlatePainter(), std::move(clicked), is_toggle);
    if (is_toggle)
    {
        plate->SetShowsLamp(true);
    }
    AddChild(*plate);
    return plate;
}

reckless::view::Button *SidePanel::plate(std::string_view text) noexcept
{
    for (auto *button : {clue_.get(), solve_.get(), check_.get(), undo_.get(), clear_.get(), pencil_marks_.get()})
    {
        if (button->text() == text)
        {
            return button;
        }
    }
    return nullptr;
}

void SidePanel::Show(const Controller &controller)
{
    headline_->SetText(controller.Headline());
    const Game &game = controller.game();
    headline_->SetColour(!game.clashes().empty() || game.Contradiction() ? theme::kBad
                         : game.IsSolved()                               ? theme::kGood
                                                                         : theme::kInk);
    message_->SetText(controller.Message());
    const bool can_run = controller.CanRun();
    clue_->SetDimmed(!can_run);
    solve_->SetDimmed(!can_run);
    undo_->SetDimmed(!game.CanUndo());
    pencil_marks_->SetOn(controller.settings().shows_pencil_marks);

    // The list follows the steps, and the step lit; telling nothing back while it is set.
    is_showing_ = true;
    std::vector<std::string> rows;
    rows.reserve(game.steps().size());
    for (std::size_t index = 0; index < game.steps().size(); ++index)
    {
        rows.push_back(std::to_string(index + 1) + ". " + game.steps()[index].description);
    }
    if (rows != steps_model_.steps())
    {
        steps_model_.SetSteps(std::move(rows));
        steps_->RowsChanged();
        shown_step_.reset();
    }
    if (controller.shown_step() != shown_step_)
    {
        shown_step_ = controller.shown_step();
        if (shown_step_)
        {
            steps_->Select(*shown_step_);
        }
        else
        {
            steps_->ClearSelection();
        }
    }
    is_showing_ = false;
}

void SidePanel::Paint(reckless::gfx::Canvas &c)
{
    c.FillAll(theme::kPanel);
    c.SetColor(theme::kRule);
    c.DrawVerticalLine(0, 0.0f, static_cast<float>(height()));
}

void SidePanel::Resized()
{
    const int inner = width() - 2 * kMargin;
    int       y     = 16;
    headline_->SetBounds({kMargin, y, inner, 40});
    y += 44;
    message_->SetBounds({kMargin, y, inner, 66});
    y += 74;

    const int section_height =
        reckless::app::Section::kTitleHeight + kPlateHeight + 2 * reckless::app::Section::kPadding;
    // A section holding one row of plates, the row split evenly.
    const auto lay_row = [&](reckless::app::Section &section, std::initializer_list<reckless::view::Button *> plates) {
        section.SetBounds({kMargin, y, inner, section_height});
        const RectI area  = section.ContentArea().Translated(section.bounds().x(), section.bounds().y());
        const int   count = static_cast<int>(plates.size());
        const int   each  = (area.width() - (count - 1) * kGap) / count;
        int         index = 0;
        for (auto *plate : plates)
        {
            plate->SetBounds(
                {area.x() + index * (each + kGap), area.y() + (area.height() - kPlateHeight) / 2, each, kPlateHeight});
            ++index;
        }
        y += section_height + 8;
    };
    lay_row(*solve_section_, {clue_.get(), solve_.get(), check_.get()});
    // Undo and Clear narrower, the switch's caption wider.
    board_section_->SetBounds({kMargin, y, inner, section_height});
    {
        const RectI area =
            board_section_->ContentArea().Translated(board_section_->bounds().x(), board_section_->bounds().y());
        const int small = (area.width() - 2 * kGap) * 3 / 10;
        const int top   = area.y() + (area.height() - kPlateHeight) / 2;
        undo_->SetBounds({area.x(), top, small, kPlateHeight});
        clear_->SetBounds({area.x() + small + kGap, top, small, kPlateHeight});
        pencil_marks_->SetBounds(
            {area.x() + 2 * (small + kGap), top, area.right() - (area.x() + 2 * (small + kGap)), kPlateHeight});
    }
    y += section_height + 8;

    const int steps_height = std::max(height() - y - kMargin, section_height);
    steps_section_->SetBounds({kMargin, y, inner, steps_height});
    const RectI area =
        steps_section_->ContentArea().Translated(steps_section_->bounds().x(), steps_section_->bounds().y());
    steps_->SetBounds(area);
}

} // namespace sudoku::app
