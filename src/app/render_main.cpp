// The window drawn to a PNG in a chosen state, for review images and the docs: `empty` (a first launch), `puzzle` (an
// X-wing puzzle just opened), `clue` (twelve clues into it, the last step lit), `solved`, `stuck` (AI Escargot, past
// every technique), `clash` (two 5s in a row), `no-marks` (the pencil marks off) and `about` (the About card); and with
// `icon <size>` the app's icon.

#include "app/about_view.h"
#include "app/board_view.h"
#include "app/controller.h"
#include "app/main_view.h"
#include "app/theme.h"
#include "solver/puzzle_file.h"

#include "AppResources.h"
#include "SkiaCanvas.h"
#include "View.h"
#include "version.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace
{

/** The X-wing puzzle (puzzles/xwing1.sdk). */
constexpr const char *kXWing = "...13...5.4....2..8..9.........5.9....2...4....3.6.........3..6..5....1.7...28...";

/** AI Escargot, which needs chains deeper than the solver follows. */
constexpr const char *kEscargot = "1....7.9..3..2...8..96..5....53..9...1..8...26....4...3......1..4......7..7...3..";

/** The window's host with no window: text measured as Skia draws it, so a line of text lays out as the window does. */
class RenderHost final : public reckless::view::Host
{
  public:
    /** Nothing to repaint: the whole window is drawn once. @param root_area The area. */
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
 * The image written to `path` as a PNG.
 * @param image The image.
 * @param path Where.
 * @return Whether it was written.
 */
bool Write(const reckless::gfx::Image &image, const std::string &path)
{
    const std::vector<std::uint8_t> png = reckless::gfx::EncodePng(image);
    std::ofstream                   file(path, std::ios::binary);
    file.write(reinterpret_cast<const char *>(png.data()), static_cast<std::streamsize>(png.size()));
    return static_cast<bool>(file);
}

/**
 * The icon: a box of the board on a rounded square - two givens in cream, a value the solver placed in amber on its
 * green wash, and an empty square's pencil marks.
 * @param size Its side, in pixels.
 * @param path Where to write it.
 * @return The exit code.
 */
int RenderIcon(int size, const std::string &path)
{
    using reckless::gfx::RectF;
    namespace theme            = sudoku::app::theme;
    reckless::gfx::Image image = reckless::gfx::MakeImage(size, size);
    {
        const auto canvas = reckless::gfx::MakeImageCanvas(image, static_cast<float>(size) / 256.0f);
        if (!canvas)
        {
            return 1;
        }
        reckless::gfx::Canvas &c = *canvas;
        reckless::gfx::Path    square;
        square.AddRoundedRect({8.0f, 8.0f, 240.0f, 240.0f}, 52.0f);
        c.ClipToPath(square);
        c.SetGradient(reckless::gfx::Gradient{
            theme::kWindow.Brighter(0.08f), {128.0f, 40.0f}, theme::kWindow.Darker(0.3f), {128.0f, 240.0f}, false});
        c.FillAll();
        const RectF board{40.0f, 40.0f, 176.0f, 176.0f};
        const float side = board.width() / 3.0f;
        c.SetColor(theme::kBoard);
        c.FillRoundedRect(board, 10.0f);
        const auto cell = [&](int row, int column) {
            return RectF{board.x() + static_cast<float>(column) * side, board.y() + static_cast<float>(row) * side,
                         side, side};
        };
        c.SetColor(theme::kGood.WithAlpha(0.28f));
        c.FillRect(cell(1, 1));
        c.SetFont(reckless::gfx::Font(side * 0.7f).Bold());
        c.SetColor(theme::kInk);
        c.DrawText("1", cell(0, 0), reckless::gfx::Justify::kCentred, false);
        c.DrawText("9", cell(2, 2), reckless::gfx::Justify::kCentred, false);
        c.SetColor(theme::kAmber);
        c.SetFont(reckless::gfx::Font(side * 0.7f));
        c.DrawText("5", cell(1, 1), reckless::gfx::Justify::kCentred, false);
        c.SetFont(sudoku::app::fonts::Mono(side * 0.3f));
        c.SetColor(theme::kMuted);
        const RectF marks = cell(0, 2);
        c.DrawText("3", RectF{marks.x(), marks.y(), side / 2.0f, side / 2.0f}, reckless::gfx::Justify::kCentred, false);
        c.DrawText("7", RectF{marks.x() + side / 2.0f, marks.bottom() - side / 2.0f, side / 2.0f, side / 2.0f},
                   reckless::gfx::Justify::kCentred, false);
        c.SetColor(theme::kThinLine);
        for (int line = 1; line < 3; ++line)
        {
            const float offset = static_cast<float>(line) * side;
            c.DrawLine({board.x() + offset, board.y()}, {board.x() + offset, board.bottom()}, 2.0f);
            c.DrawLine({board.x(), board.y() + offset}, {board.right(), board.y() + offset}, 2.0f);
        }
        c.SetColor(theme::kAmber);
        c.DrawRoundedRect(board, 10.0f, 6.0f);
    }
    return Write(image, path) ? 0 : 1;
}

/**
 * @param text A puzzle as text.
 * @return Its values.
 */
sudoku::Values Puzzle(const char *text)
{
    return sudoku::puzzle_file::Parse(text);
}

} // namespace

/**
 * Draw the window, or the icon, into a PNG.
 * @param argc The argument count.
 * @param argv The arguments.
 * @return 0 when the file was written, 1 otherwise, 2 for a usage error.
 */
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "usage: sudoku_render <out.png> [state] [scale]\n"
                             "  state: empty puzzle clue solved stuck clash no-marks about\n"
                             "       sudoku_render <out.png> icon <size>\n");
        return 2;
    }
    sudoku::app::fonts::InstallFaces();
    const std::string out   = argv[1];
    const std::string state = argc > 2 ? argv[2] : "clue";
    if (state == "icon")
    {
        return RenderIcon(argc > 3 ? std::atoi(argv[3]) : 256, out);
    }
    const float scale = argc > 3 ? static_cast<float>(std::atof(argv[3])) : 1.0f;

    sudoku::app::Controller controller;
    controller.Select(sudoku::geometry::Cell(4, 4));
    if (state == "puzzle" || state == "clue" || state == "solved" || state == "no-marks")
    {
        const sudoku::Values puzzle = Puzzle(kXWing);
        for (int cell = 0; cell < sudoku::geometry::kCellCount; ++cell)
        {
            if (puzzle[static_cast<std::size_t>(cell)] != 0)
            {
                controller.Enter(cell, puzzle[static_cast<std::size_t>(cell)]);
            }
        }
        if (state == "clue" || state == "no-marks")
        {
            for (int clue = 0; clue < 12; ++clue)
            {
                controller.Clue();
            }
        }
        if (state == "solved")
        {
            controller.Solve();
        }
        controller.SetPencilMarks(state != "no-marks");
    }
    else if (state == "stuck")
    {
        const sudoku::Values puzzle = Puzzle(kEscargot);
        for (int cell = 0; cell < sudoku::geometry::kCellCount; ++cell)
        {
            if (puzzle[static_cast<std::size_t>(cell)] != 0)
            {
                controller.Enter(cell, puzzle[static_cast<std::size_t>(cell)]);
            }
        }
        controller.Solve();
    }
    else if (state == "clash")
    {
        controller.Enter(sudoku::geometry::Cell(0, 0), 5);
        controller.Enter(sudoku::geometry::Cell(0, 8), 5);
        controller.Select(sudoku::geometry::Cell(0, 8));
    }

    // The host before the view, so it outlives it: the view tells its host as it goes.
    RenderHost                        host;
    const reckless::app::AppResources resources(sudoku::kVersion);
    sudoku::app::MainView             view(resources);
    view.SetHost(&host);
    view.SetBounds({sudoku::app::MainView::kDesignWidth, sudoku::app::MainView::kDesignHeight});
    view.Show(controller);
    std::unique_ptr<sudoku::app::AboutView> about;
    if (state == "about")
    {
        about = std::make_unique<sudoku::app::AboutView>(sudoku::kVersion);
        view.SetOverlay(about.get());
    }
    const int width  = static_cast<int>(static_cast<float>(sudoku::app::MainView::kDesignWidth) * scale + 0.5f);
    const int height = static_cast<int>(static_cast<float>(sudoku::app::MainView::kDesignHeight) * scale + 0.5f);
    reckless::gfx::Image image = reckless::gfx::MakeImage(width, height);
    {
        const auto canvas = reckless::gfx::MakeImageCanvas(image, scale);
        if (!canvas)
        {
            std::fprintf(stderr, "sudoku_render: no canvas\n");
            return 1;
        }
        reckless::view::PaintTree(view, *canvas);
    }
    if (!Write(image, out))
    {
        std::fprintf(stderr, "sudoku_render: could not write %s\n", out.c_str());
        return 1;
    }
    std::printf("%s: %d x %d at %.2fx\n", out.c_str(), width, height, static_cast<double>(scale));
    return 0;
}
