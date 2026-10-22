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

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
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

/** A popup that only records what it shows: it lies nowhere on the screen. */
class ShownPopup final : public reckless::view::Popup
{
  public:
    /**
     * Whether a point lies in its window: never.
     * @param screen The point.
     * @return False.
     */
    bool Contains(reckless::gfx::PointI screen) const override
    {
        static_cast<void>(screen);
        return false;
    }
};

/**
 * The window's host with popups and a loop of its own: the last popup's content kept, and what the views ask to run
 * soon held until it is drained, as the window holds it until the event that asked is over.
 */
class PopupHost final : public reckless::view::Host
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

    /**
     * A popup showing `content`, kept as the last one opened.
     * @param content The view the popup shows.
     * @param at Its top left.
     * @param pressed_outside Run on a press outside it: never here.
     * @param takes_mouse Whether it takes the mouse.
     * @return The popup.
     */
    std::unique_ptr<reckless::view::Popup> OpenPopup(reckless::view::View &content, reckless::gfx::PointI at,
                                                     std::function<void(reckless::gfx::PointI)> pressed_outside,
                                                     bool                                       takes_mouse) override
    {
        static_cast<void>(at);
        static_cast<void>(pressed_outside);
        static_cast<void>(takes_mouse);
        shown = &content;
        return std::make_unique<ShownPopup>();
    }

    /** Held until Drain. @param call What to run. */
    void CallSoon(std::function<void()> call) override
    {
        soon.push_back(std::move(call));
    }

    /** Runs what was asked to run soon, in order, until nothing is left. */
    void Drain()
    {
        while (!soon.empty())
        {
            std::function<void()> call = std::move(soon.front());
            soon.erase(soon.begin());
            call();
        }
    }

    reckless::view::View              *shown{nullptr}; ///< The last popup's content.
    std::vector<std::function<void()>> soon;           ///< What was asked to run soon.
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

/** A board view 600 x 600: the board 528 across from (36, 36), each square 528 / 9 across. */
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
        const float side = 528.0f / 9.0f;
        return {36.0f + (static_cast<float>(geometry::Column(cell)) + 0.5f) * side,
                36.0f + (static_cast<float>(geometry::Row(cell)) + 0.5f) * side};
    }

    BoardView              board_;    ///< The board.
    std::vector<int>       selected_; ///< The squares told.
    std::vector<Candidate> digits_;   ///< The digits told, as square and digit.
};

TEST_F(Board, FitsTheLargestSquareItsViewHoldsCentred)
{
    const reckless::gfx::RectF area = board_.BoardArea();
    EXPECT_FLOAT_EQ(area.x(), 36.0f);
    EXPECT_FLOAT_EQ(area.width(), 528.0f);
    board_.SetBounds({0, 0, 900, 600});
    const reckless::gfx::RectF wide = board_.BoardArea();
    EXPECT_FLOAT_EQ(wide.width(), 528.0f);
    EXPECT_FLOAT_EQ(wide.x(), 186.0f);
    EXPECT_FLOAT_EQ(wide.y(), 36.0f);
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
    EXPECT_EQ(board_.CellAt({300.0f, 580.0f}), -1) << "the columns' letters";
    EXPECT_EQ(board_.CellAt({20.0f, 300.0f}), -1) << "the rows' numbers";
    EXPECT_EQ(board_.CellAt({563.9f, 563.9f}), 80) << "the last pixel";
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

TEST_F(Board, ARightClickOffersTheDigitsAndClearForTheSquare)
{
    PopupHost host;
    board_.SetHost(&host);
    const int    cell = geometry::Cell(2, 5);
    BoardPicture picture;
    picture.selected = cell; // as the controller has it once the click chose the square
    board_.SetPicture(picture);

    // Each choice of the menu: 5, then Clear, each told for the square once the event that chose it is over.
    for (const int row : {4, 10})
    {
        host.shown = nullptr;
        board_.MouseDown(At(Middle(cell).x, Middle(cell).y, true));
        ASSERT_NE(host.shown, nullptr) << "no menu opened";
        auto &menu = static_cast<reckless::view::Menu &>(*host.shown);
        ASSERT_EQ(menu.items().size(), 11u) << "1 to 9, a rule and Clear";
        EXPECT_EQ(menu.items()[4].text, "5");
        EXPECT_TRUE(menu.items()[9].separator);
        EXPECT_EQ(menu.items()[10].text, "Clear");
        const reckless::gfx::RectI chosen = menu.RowBounds(static_cast<std::size_t>(row));
        menu.MouseUp(At(static_cast<float>(chosen.x() + 5), static_cast<float>(chosen.y() + 5)));
        host.Drain();
    }
    EXPECT_EQ(selected_, (std::vector<int>{cell, cell}));
    EXPECT_EQ(digits_, (std::vector<Candidate>{{cell, 5}, {cell, 0}}));

    // A left click chooses the square and opens no menu.
    host.shown = nullptr;
    board_.MouseDown(At(Middle(cell).x, Middle(cell).y));
    EXPECT_EQ(host.shown, nullptr);
    board_.SetHost(&host_);
}

TEST_F(Board, ATooSmallBoardIsNotDrawn)
{
    board_.SetBounds({0, 0, 40, 40});
    const reckless::gfx::Image image = Painted(board_);
    ASSERT_TRUE(image.IsValid());
    EXPECT_EQ(image.Bounds().width(), 40);
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
    EXPECT_EQ(panel.message(), "The givens clash: 5 is twice in row 9 (a9, i9).");
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

    auto menus = std::make_shared<reckless::app::MenuBarController>();
    menus->Set(
        reckless::app::MenuBar{{reckless::app::MenuEntry::Menu("File", {reckless::app::MenuEntry::Item("Open", 1)})}},
        {});
    view.SetMenuBarView(std::make_unique<reckless::app::MenuBarView>(menus, theme::MenuBarLook()));
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
    AboutView                about(kVersion);
    int                      closes = 0;
    std::vector<std::string> opened;
    about.on_close = [&] { ++closes; };
    view.SetOverlay(&about);
    EXPECT_EQ(view.overlay(), &about);
    EXPECT_EQ(about.bounds(), view.LocalBounds());
    EXPECT_EQ(about.Lines(), (std::vector<std::string>{
                                 "Reckless Sudoku", std::string("Version ") + kVersion,
                                 "Copyright 2008-2026 Jonny Reckless. Reckless Technology.", "recklesstechnology.com",
                                 "Free, proprietary software, used under its end-user licence (EULA.txt).",
                                 "Third-party notices are in THIRD-PARTY-NOTICES.txt, installed beside the "
                                 "app."}));
    EXPECT_TRUE(Painted(view).IsValid());

    // The website's link, lit under the pointer and opened when clicked; nothing opened with no one to open it.
    LinkLabel &link = about.link();
    EXPECT_TRUE(about.CardArea().Contains(link.bounds()));
    EXPECT_GT(link.width(), 0);
    const reckless::view::MouseEvent middle =
        At(static_cast<float>(link.width()) / 2.0f, static_cast<float>(link.height()) / 2.0f);
    link.MouseUp(middle);
    EXPECT_TRUE(opened.empty());
    about.open_url = [&](const std::string &url) { opened.push_back(url); };
    link.MouseEnter(middle);
    EXPECT_TRUE(link.is_over());
    EXPECT_TRUE(Painted(view).IsValid());
    link.MouseUp(middle);
    link.MouseUp(At(-5.0f, -5.0f)); // released outside: nothing
    link.MouseExit(middle);
    EXPECT_FALSE(link.is_over());
    EXPECT_EQ(opened, std::vector<std::string>{AboutView::kWebsite});
    EXPECT_FALSE(about.KeyPressed(KeyOf(reckless::view::Key::kCharacter, U'x')));

    EXPECT_TRUE(about.KeyPressed(KeyOf(reckless::view::Key::kEscape)));
    about.MouseDown(At(2.0f, 2.0f)); // outside the card
    EXPECT_EQ(closes, 2);
    view.SetOverlay(nullptr);
    EXPECT_EQ(view.overlay(), nullptr);
}

} // namespace
} // namespace sudoku::app
