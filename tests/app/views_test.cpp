// The window's views with no window: the board drawn and driven by the mouse and the keys, the panel's plates and its
// steps list, the main view's header and layout, the drawn menu bar, the About card, and the window held at the View
// menu's sizes.

#include "app/about_view.h"
#include "app/board_view.h"
#include "app/controller.h"
#include "app/main_view.h"
#include "app/side_panel.h"
#include "app/theme.h"
#include "solver/puzzle_file.h"
#include "version.h"

#include "AppResources.h"
#include "Button.h"
#include "LookMetrics.h"
#include "MenuBarView.h"
#include "SkiaCanvas.h"
#include "View.h"

#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace sudoku::app
{
namespace
{

/** The X-wing puzzle (puzzles/xwing1.sdk). */
constexpr const char *kXWing = "...13...5.4....2..8..9.........5.9....2...4....3.6.........3..6..5....1.7...28...";

/** The window's host with no window: text measured as Skia draws it. */
class TestHost final : public reckless::view::Host
{
  public:
    /** Nothing to repaint. @param root_area The area. */
    void Invalidate(reckless::gfx::RectI root_area) override
    {
        static_cast<void>(root_area);
    }

    /**
     * Text measured as Skia draws it.
     * @param font The font.
     * @param text The text.
     * @return Its width.
     */
    float TextWidth(const reckless::gfx::Font &font, std::string_view text) const override
    {
        return reckless::gfx::MeasureText(font, text);
    }
};

/**
 * A view tree drawn into an image.
 * @param view The view, laid out.
 * @return The image; invalid where nothing could be drawn.
 */
reckless::gfx::Image Painted(reckless::view::View &view)
{
    reckless::gfx::Image image = reckless::gfx::MakeImage(view.width(), view.height());
    {
        const auto canvas = reckless::gfx::MakeImageCanvas(image, 1.0f);
        if (!canvas)
        {
            return {};
        }
        reckless::view::PaintTree(view, *canvas);
    }
    return image;
}

/** A mouse event at a point. @param x Its x. @param y Its y. @param right Whether the secondary button. @return It. */
reckless::view::MouseEvent At(float x, float y, bool right = false)
{
    reckless::view::MouseEvent event;
    event.position = {x, y};
    event.down     = event.position;
    event.right    = right;
    return event;
}

/** A key. @param key Which. @param character Its character, for kCharacter. @return The event. */
reckless::view::KeyEvent KeyOf(reckless::view::Key key, char32_t character = 0)
{
    reckless::view::KeyEvent event;
    event.key       = key;
    event.character = character;
    return event;
}

/**
 * A press and a release in the middle of a button.
 * @param button The button.
 */
void Press(reckless::view::Button &button)
{
    const reckless::view::MouseEvent event =
        At(static_cast<float>(button.width()) / 2.0f, static_cast<float>(button.height()) / 2.0f);
    button.MouseDown(event);
    button.MouseUp(event);
}

/** The faces handed to Skia once, as the app does, so text lays out as it draws. */
class Views : public ::testing::Test
{
  protected:
    static void SetUpTestSuite()
    {
        fonts::InstallFaces();
    }

    TestHost host_; ///< The views' host.
};

/** A board view 600 x 600: the board 552 across from (24, 24), each square 552 / 9 across. */
class Board : public Views
{
  protected:
    void SetUp() override
    {
        board_.SetHost(&host_);
        board_.SetBounds({0, 0, 600, 600});
        board_.on_select = [this](int cell) { selected_.push_back(cell); };
        board_.on_digit  = [this](int cell, int digit) { digits_.push_back({cell, digit}); };
    }

    /** @param cell A square. @return The view's point at its middle. */
    static reckless::gfx::PointF Middle(int cell)
    {
        const float side = 552.0f / 9.0f;
        return {24.0f + (static_cast<float>(geometry::Column(cell)) + 0.5f) * side,
                24.0f + (static_cast<float>(geometry::Row(cell)) + 0.5f) * side};
    }

    BoardView              board_;    ///< The board.
    std::vector<int>       selected_; ///< The squares told.
    std::vector<Candidate> digits_;   ///< The digits told, as square and digit.
};

TEST_F(Board, FitsTheLargestSquareItsViewHoldsCentred)
{
    const reckless::gfx::RectF area = board_.BoardArea();
    EXPECT_FLOAT_EQ(area.x(), 24.0f);
    EXPECT_FLOAT_EQ(area.width(), 552.0f);
    board_.SetBounds({0, 0, 900, 600});
    const reckless::gfx::RectF wide = board_.BoardArea();
    EXPECT_FLOAT_EQ(wide.width(), 552.0f);
    EXPECT_FLOAT_EQ(wide.x(), 174.0f);
    EXPECT_FLOAT_EQ(wide.y(), 24.0f);
    board_.SetBounds({0, 0, 30, 30});
    EXPECT_FLOAT_EQ(board_.BoardArea().width(), 0.0f);
    EXPECT_EQ(board_.CellAt({15.0f, 15.0f}), -1);
}

TEST_F(Board, FindsTheSquareUnderAPoint)
{
    for (const int cell : {0, 8, 40, 72, 80})
    {
        const reckless::gfx::PointF middle = Middle(cell);
        EXPECT_EQ(board_.CellAt(middle), cell);
    }
    EXPECT_EQ(board_.CellAt({10.0f, 300.0f}), -1) << "the margin";
    EXPECT_EQ(board_.CellAt({300.0f, 590.0f}), -1);
    EXPECT_EQ(board_.CellAt({575.9f, 575.9f}), 80) << "the last pixel";
}

TEST_F(Board, AClickChoosesTheSquareUnderIt)
{
    board_.MouseDown(At(Middle(23).x, Middle(23).y));
    board_.MouseDown(At(5.0f, 5.0f)); // off the board: nothing
    EXPECT_EQ(selected_, std::vector<int>{23});
    EXPECT_TRUE(board_.WantsKeyboardFocus());
    EXPECT_TRUE(board_.KeepsKeyboardFocus());
}

TEST_F(Board, TheKeysMoveTheChoiceAndEnterDigits)
{
    using reckless::view::Key;
    BoardPicture picture;
    picture.selected = geometry::Cell(0, 0);
    board_.SetPicture(picture);
    EXPECT_TRUE(board_.KeyPressed(KeyOf(Key::kUp)));
    EXPECT_TRUE(board_.KeyPressed(KeyOf(Key::kLeft)));
    EXPECT_TRUE(board_.KeyPressed(KeyOf(Key::kDown)));
    EXPECT_TRUE(board_.KeyPressed(KeyOf(Key::kRight)));
    EXPECT_EQ(selected_, (std::vector<int>{0, 0, geometry::Cell(1, 0), geometry::Cell(0, 1)})) << "held to the board";
    for (const char32_t typed : {U'1', U'9', U'0', U' '})
    {
        EXPECT_TRUE(board_.KeyPressed(KeyOf(Key::kCharacter, typed)));
    }
    EXPECT_TRUE(board_.KeyPressed(KeyOf(Key::kBackspace)));
    EXPECT_TRUE(board_.KeyPressed(KeyOf(Key::kDelete)));
    EXPECT_EQ(digits_, (std::vector<Candidate>{{0, 1}, {0, 9}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}));
    EXPECT_FALSE(board_.KeyPressed(KeyOf(Key::kCharacter, U'x')));
    EXPECT_FALSE(board_.KeyPressed(KeyOf(Key::kReturn)));
    reckless::view::KeyEvent command = KeyOf(Key::kCharacter, U'5');
    command.modifiers.command        = true;
    EXPECT_FALSE(board_.KeyPressed(command)) << "a shortcut is the menu's";
    EXPECT_EQ(digits_.size(), 6u);
}

TEST_F(Board, ListensToNoOneWhenNoOneListens)
{
    BoardView quiet;
    quiet.SetHost(&host_);
    quiet.SetBounds({0, 0, 600, 600});
    quiet.MouseDown(At(300.0f, 300.0f));
    EXPECT_TRUE(quiet.KeyPressed(KeyOf(reckless::view::Key::kCharacter, U'5')));
    EXPECT_TRUE(quiet.KeyPressed(KeyOf(reckless::view::Key::kDown)));
}

TEST_F(Board, PaintsEveryKindOfSquare)
{
    Controller   controller;
    const Values puzzle = puzzle_file::Parse(kXWing);
    controller.Enter(0, 0);
    for (int cell = 0; cell < geometry::kCellCount; ++cell)
    {
        if (puzzle[static_cast<std::size_t>(cell)] != 0)
        {
            controller.Enter(cell, puzzle[static_cast<std::size_t>(cell)]);
        }
    }
    for (int clue = 0; clue < 12; ++clue)
    {
        controller.Clue();
    }
    BoardPicture picture = controller.Picture();
    picture.clashes      = {1, 2};
    board_.SetPicture(picture);
    board_.FocusGained();
    const reckless::gfx::Image focused = Painted(board_);
    ASSERT_TRUE(focused.IsValid());
    EXPECT_EQ(focused.Bounds().width(), 600);
    board_.FocusLost();
    picture.shows_pencil_marks = false;
    board_.SetPicture(picture);
    EXPECT_EQ(board_.picture(), picture);
    EXPECT_TRUE(Painted(board_).IsValid());
}

TEST_F(Views, ThePanelTellsEveryPlateAndDimsWhatCannotBeDone)
{
    SidePanel panel;
    panel.SetHost(&host_);
    panel.SetBounds({0, 0, SidePanel::kWidth, 700});
    std::vector<std::string> told;
    panel.on_clue         = [&] { told.push_back("clue"); };
    panel.on_solve        = [&] { told.push_back("solve"); };
    panel.on_check        = [&] { told.push_back("check"); };
    panel.on_undo         = [&] { told.push_back("undo"); };
    panel.on_clear        = [&] { told.push_back("clear"); };
    panel.on_pencil_marks = [&](bool shows) { told.push_back(shows ? "marks on" : "marks off"); };

    Controller controller;
    controller.Enter(0, 5);
    panel.Show(controller);
    EXPECT_EQ(panel.headline(), "1 of 81");
    EXPECT_FALSE(panel.plate("Undo")->dimmed());
    EXPECT_TRUE(panel.plate("Pencil marks")->on());
    for (const char *caption : {"Clue", "Solve", "Check", "Undo", "Clear", "Pencil marks"})
    {
        ASSERT_NE(panel.plate(caption), nullptr) << caption;
        Press(*panel.plate(caption));
    }
    EXPECT_EQ(told, (std::vector<std::string>{"clue", "solve", "check", "undo", "clear", "marks off"}));
    EXPECT_EQ(panel.plate("Nothing"), nullptr);

    controller.Enter(8, 5);
    panel.Show(controller);
    EXPECT_EQ(panel.headline(), "Givens clash");
    EXPECT_TRUE(panel.plate("Clue")->dimmed());
    EXPECT_TRUE(panel.plate("Solve")->dimmed());
    EXPECT_EQ(panel.message(), "The givens clash: 5 is twice in row 1 (r1c1, r1c9).");
    EXPECT_TRUE(Painted(panel).IsValid());
}

TEST_F(Views, TheStepsListFollowsTheStepsAndLightsTheOneChosen)
{
    SidePanel panel;
    panel.SetHost(&host_);
    panel.SetBounds({0, 0, SidePanel::kWidth, 700});
    std::vector<std::optional<std::size_t>> chosen;
    panel.on_step = [&](std::optional<std::size_t> index) { chosen.push_back(index); };

    Controller   controller;
    const Values puzzle = puzzle_file::Parse(kXWing);
    for (int cell = 0; cell < geometry::kCellCount; ++cell)
    {
        if (puzzle[static_cast<std::size_t>(cell)] != 0)
        {
            controller.Enter(cell, puzzle[static_cast<std::size_t>(cell)]);
        }
    }
    controller.Clue();
    controller.Clue();
    panel.Show(controller);
    const std::size_t count = controller.game().steps().size();
    ASSERT_EQ(panel.steps_model().RowCount(), count);
    EXPECT_EQ(panel.steps_model().CellText(0, 0), "1. " + controller.game().steps()[0].description);
    EXPECT_EQ(panel.steps_model().CellText(count, 0), "");
    EXPECT_TRUE(panel.steps().IsSelected(count - 1)) << "the last step lit";
    EXPECT_TRUE(chosen.empty()) << "nothing told back while the panel is set";

    panel.steps().Select(0);
    ASSERT_EQ(chosen.size(), 1u);
    EXPECT_EQ(chosen.back(), std::optional<std::size_t>(0));
    controller.ShowStep(0);
    panel.Show(controller);
    EXPECT_TRUE(panel.steps().IsSelected(0));

    controller.Undo();
    controller.Undo();
    panel.Show(controller);
    EXPECT_EQ(panel.steps_model().RowCount(), 0u);
}

TEST_F(Views, TheMainViewDrawsTheHeaderAndLaysOutTheBoardAndThePanel)
{
    const reckless::app::AppResources resources(kVersion);
    MainView                          view(resources);
    view.SetHost(&host_);
    view.SetBounds({MainView::kDesignWidth, MainView::kDesignHeight});
    const int header = reckless::look::kHeaderHeight;
    EXPECT_EQ(view.board().bounds().y(), header);
    EXPECT_EQ(view.board().bounds().width(), MainView::kDesignWidth - SidePanel::kWidth);
    EXPECT_EQ(view.panel().bounds().x(), MainView::kDesignWidth - SidePanel::kWidth);
    EXPECT_EQ(view.panel().bounds().height(), MainView::kDesignHeight - header);
    Controller controller;
    view.Show(controller);
    EXPECT_TRUE(Painted(view).IsValid());

    view.SetMenuBarView(std::make_unique<reckless::app::MenuBarView>(theme::MenuBarLook()));
    EXPECT_EQ(view.board().bounds().y(), header + theme::MenuBarLook().height) << "the drawn bar above the header";
    EXPECT_TRUE(Painted(view).IsValid());
    view.SetMenuBarView(nullptr);
    EXPECT_EQ(view.board().bounds().y(), header);
}

TEST_F(Views, ShowsTheAboutCardAndClosesIt)
{
    const reckless::app::AppResources resources(kVersion);
    MainView                          view(resources);
    view.SetHost(&host_);
    view.SetBounds({MainView::kDesignWidth, MainView::kDesignHeight});
    AboutView about(kVersion, "The licence.", "The notices.");
    int       closes = 0;
    about.on_close   = [&] { ++closes; };
    view.SetOverlay(&about);
    EXPECT_EQ(view.overlay(), &about);
    EXPECT_EQ(about.bounds(), view.LocalBounds());
    EXPECT_EQ(about.text(), "The licence.\n\nThe notices.");
    EXPECT_TRUE(Painted(view).IsValid());
    EXPECT_TRUE(about.KeyPressed(KeyOf(reckless::view::Key::kEscape)));
    about.MouseDown(At(2.0f, 2.0f)); // outside the card
    EXPECT_EQ(closes, 2);
    view.SetOverlay(nullptr);
    EXPECT_EQ(view.overlay(), nullptr);

    AboutView embedded(kVersion);
    EXPECT_FALSE(embedded.text().empty()) << "the licence and the notices the binary carries";
}

} // namespace
} // namespace sudoku::app
