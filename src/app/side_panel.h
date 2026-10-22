#pragma once
/// @file side_panel.h The panel beside the board: where the puzzle stands and what was last done, the solver's plates,
/// undo and clear, the pencil marks' switch, and the steps taken - choosing one lights it on the board.

#include "app/controller.h"

#include "AppWidgets.h"
#include "Button.h"
#include "ListView.h"
#include "View.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace sudoku::app
{

/** A message over up to three lines, cut short with an ellipsis past them. */
class MessageView final : public reckless::view::View
{
  public:
    MessageView();

    /** @param text What to say; repainted when it differs. */
    void SetText(std::string text);

    /** @return What it says. */
    const std::string &text() const noexcept
    {
        return text_;
    }

    void Paint(reckless::gfx::Canvas &c) override;

  private:
    std::string text_; ///< What it says.
};

/** The steps as the list shows them: "12. Hidden pair {3,7} in row 4 ...". */
class StepsModel final : public reckless::view::ListModel
{
  public:
    /** @param steps The steps; set again when they change. */
    void SetSteps(std::vector<std::string> steps)
    {
        steps_ = std::move(steps);
    }

    /** @return The rows' text. */
    const std::vector<std::string> &steps() const noexcept
    {
        return steps_;
    }

    std::size_t RowCount() const override
    {
        return steps_.size();
    }

    std::string CellText(std::size_t row, std::size_t column) const override;

  private:
    std::vector<std::string> steps_; ///< Each step's line.
};

/**
 * The panel: a headline of where the puzzle stands and a message of what was last done, then sections of plates - Clue,
 * Solve and Check; Undo and Clear; the pencil marks' switch - and the steps list. Show() sets it from the controller
 * each frame; the `on_*` callbacks tell what the person chose.
 */
class SidePanel final : public reckless::view::View
{
  public:
    static constexpr int kWidth = 380; ///< The panel across, in design pixels.

    SidePanel();

    std::function<void()>                           on_clue;         ///< "Clue".
    std::function<void()>                           on_solve;        ///< "Solve".
    std::function<void()>                           on_check;        ///< "Check".
    std::function<void()>                           on_undo;         ///< "Undo".
    std::function<void()>                           on_clear;        ///< "Clear".
    std::function<void(bool shows)>                 on_pencil_marks; ///< The pencil marks switched.
    std::function<void(std::optional<std::size_t>)> on_step;         ///< A step chosen in the list; none for none.

    /**
     * The panel as the controller stands.
     * @param controller The window's state.
     */
    void Show(const Controller &controller);

    /** @return The headline shown. */
    const std::string &headline() const noexcept
    {
        return headline_->text();
    }

    /** @return The message shown. */
    const std::string &message() const noexcept
    {
        return message_->text();
    }

    /** @return The steps list. */
    reckless::view::ListView &steps() noexcept
    {
        return *steps_;
    }

    /** @return The steps as the list shows them. */
    const StepsModel &steps_model() const noexcept
    {
        return steps_model_;
    }

    /**
     * @param text A plate's caption: "Clue", "Solve", "Check", "Undo", "Clear" or "Pencil marks".
     * @return The plate; null for another caption.
     */
    reckless::view::Button *plate(std::string_view text) noexcept;

    void Paint(reckless::gfx::Canvas &c) override;
    void Resized() override;

  private:
    /**
     * A plate that does something.
     * @param text Its caption.
     * @param clicked What it does.
     * @param is_toggle Whether it is a switch with a lamp, on or off.
     * @return The plate, added to the panel.
     */
    std::unique_ptr<reckless::view::Button> AddPlate(std::string text, std::function<void()> clicked,
                                                     bool is_toggle = false);

    std::unique_ptr<reckless::app::TextLine>  headline_;          ///< Where the puzzle stands.
    std::unique_ptr<MessageView>              message_;           ///< What was last done.
    std::unique_ptr<reckless::app::Section>   solve_section_;     ///< "Solve".
    std::unique_ptr<reckless::app::Section>   board_section_;     ///< "Board".
    std::unique_ptr<reckless::app::Section>   steps_section_;     ///< "Steps".
    std::unique_ptr<reckless::view::Button>   clue_;              ///< "Clue".
    std::unique_ptr<reckless::view::Button>   solve_;             ///< "Solve".
    std::unique_ptr<reckless::view::Button>   check_;             ///< "Check".
    std::unique_ptr<reckless::view::Button>   undo_;              ///< "Undo".
    std::unique_ptr<reckless::view::Button>   clear_;             ///< "Clear".
    std::unique_ptr<reckless::view::Button>   pencil_marks_;      ///< "Pencil marks", a switch.
    StepsModel                                steps_model_;       ///< The list's rows.
    std::unique_ptr<reckless::view::ListView> steps_;             ///< The steps list.
    std::optional<std::size_t>                shown_step_;        ///< The step the list has chosen, as last set.
    bool                                      is_showing_{false}; ///< Show() is setting the list: no on_step.
};

} // namespace sudoku::app
