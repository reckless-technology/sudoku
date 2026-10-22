// Reckless Sudoku: the window on the machine's own platform, through the toolkit's app shell (AppRun.h, AppRoot.h)
// and its child window on Skia. A puzzle's file named on the command line opens with the window. `--smoke [frames]` is
// a launch's check: the window opened, a few frames drawn, closed, with nothing read or kept - exit 0 when every frame
// was drawn, 3 when too few were, 1 where no window could be shown.

#include "app/about_view.h"
#include "app/controller.h"
#include "app/main_view.h"
#include "app/menus.h"
#include "app/paths.h"
#include "app/puzzle_bank_folder.h"
#include "app/settings_file.h"
#include "app/theme.h"
#include "version.h"

#include "AppPlatform.h"
#include "AppResources.h"
#include "AppRoot.h"
#include "AppRun.h"
#include "AppWindowSizes.h"
#include "Embedded.h"
#include "Window.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

/**
 * The app's window.
 * @param argc The argument count.
 * @param argv The arguments.
 * @return The exit code (reckless::app::RunApp).
 */
int main(int argc, char **argv)
{
    using sudoku::app::MainView;
    reckless::app::AppOptions options = reckless::app::ParseAppOptions(std::vector<std::string>(argv + 1, argv + argc));
    // The first argument the shell does not know is a puzzle to open.
    std::optional<std::filesystem::path> puzzle;
    if (!options.unknown.empty() && !options.unknown.front().starts_with("-"))
    {
        puzzle = sudoku::app::PathFromUtf8(options.unknown.front());
        options.unknown.erase(options.unknown.begin());
    }
    sudoku::app::fonts::InstallFaces();

    const std::unique_ptr<reckless::app::AppPlatform> platform =
        reckless::app::AppPlatform::Create("Reckless Sudoku", reckless::embedded::Find("icon_256.png"));
    const reckless::app::AppResources resources(sudoku::kVersion);

    // The person's choices as they left them (settings_file.h); a launch's check reads and keeps none.
    sudoku::app::SettingsKeeper keeper(options.is_smoke ? std::filesystem::path() : sudoku::app::DefaultSettingsFile());
    sudoku::app::Controller controller(keeper.Read(), sudoku::app::PuzzleBankBeside(sudoku::app::ExecutableFolder()));
    if (puzzle)
    {
        controller.Open(*puzzle);
    }

    MainView view(resources);
    if (!platform->HasNativeMenuBar())
    {
        view.SetMenuBarView(platform->MakeMenuBarView(sudoku::app::theme::MenuBarLook()));
    }
    reckless::app::AppRoot     root(view, sudoku::app::theme::Shell().Tooltip(), {},
                                    {MainView::kLayoutWidth, MainView::kLayoutHeight});
    reckless::app::WindowSizes sizes(root, {MainView::kDesignWidth, MainView::kDesignHeight});
    sizes.Restore(controller.settings().window_percent);

    // Help > About: a card over the window, made when asked for and gone when closed.
    std::unique_ptr<sudoku::app::AboutView> about;
    const auto                              show_about = [&] {
        about           = std::make_unique<sudoku::app::AboutView>(sudoku::kVersion);
        about->open_url = [](const std::string &url) { reckless::window::OpenInBrowser(url); };
        about->on_close = [&] {
            view.SetOverlay(nullptr);
            platform->CallSoon([&] { about.reset(); });
            view.board().GrabKeyboardFocus();
        };
        view.SetOverlay(about.get());
        about->GrabKeyboardFocus();
    };
    // File > Open and Save: the platform's dialog, the choice handed to the controller.
    std::string folder;
    const auto  choose = [&](bool is_save) {
        reckless::view::FileRequest request;
        request.title     = is_save ? "Save the puzzle's givens" : "Open a puzzle";
        request.folder    = folder;
        request.patterns  = {"*.sud", "*.sdk"};
        request.save      = is_save;
        request.file_name = controller.file_name().empty() ? "puzzle.sud" : controller.file_name();
        view.ChooseFile(request, [&, is_save](const std::string &path) {
            if (path.empty())
            {
                return;
            }
            const std::filesystem::path chosen = sudoku::app::PathFromUtf8(path);
            folder                             = sudoku::app::Utf8FromPath(chosen.parent_path());
            if (is_save)
            {
                controller.Save(chosen);
            }
            else
            {
                controller.Open(chosen);
            }
            view.board().GrabKeyboardFocus();
        });
    };
    sudoku::app::CommandHandlers handlers;
    handlers.open        = [&] { choose(false); };
    handlers.save        = [&] { choose(true); };
    handlers.quit        = [&platform] { platform->CallSoon([&platform] { platform->Quit(); }); };
    handlers.about       = show_about;
    handlers.window_size = [&](int percent) {
        if (sizes.Choose(percent))
        {
            controller.SetWindowPercent(percent);
        }
    };

    // Whenever the choices or what the menu can do change: kept, and the menu bar set again; the title follows the
    // file.
    std::optional<sudoku::app::MenuState> menu_state;
    reckless::app::AppWindow             *window = nullptr;
    std::string                           title;
    const auto                            follow = [&] {
        keeper.Changed(controller.settings());
        const sudoku::app::MenuState now = sudoku::app::MenuStateOf(controller, sizes.Menu());
        if (menu_state != now)
        {
            menu_state = now;
            platform->SetMenuBar(sudoku::app::BuildMenuBar(now), [&](int command) {
                sudoku::app::RunCommand(command, controller, handlers, sizes.Menu());
            });
        }
        const std::string now_title =
            controller.file_name().empty() ? "Reckless Sudoku" : controller.file_name() + " - Reckless Sudoku";
        if (window != nullptr && now_title != title)
        {
            title = now_title;
            window->SetTitle(title);
        }
    };

    sudoku::app::BoardView &board = view.board();
    board.on_select               = [&](int cell) { controller.Select(cell); };
    board.on_digit                = [&](int cell, int digit) { controller.Enter(cell, digit); };
    sudoku::app::SidePanel &panel = view.panel();
    panel.on_new_puzzle           = [&](sudoku::puzzle_bank::Band band) { controller.NewPuzzle(band); };
    panel.on_clue                 = [&] { sudoku::app::RunCommand(sudoku::app::kClue, controller, handlers, {}); };
    panel.on_solve                = [&] { sudoku::app::RunCommand(sudoku::app::kSolve, controller, handlers, {}); };
    panel.on_check                = [&] { controller.Check(); };
    panel.on_undo                 = [&] { controller.Undo(); };
    panel.on_clear                = [&] { controller.Clear(); };
    panel.on_pencil_marks         = [&](bool shows) { controller.SetPencilMarks(shows); };
    panel.on_step                 = [&](std::optional<std::size_t> index) { controller.ShowStep(index); };

    reckless::app::AppRunHooks hooks;
    hooks.has_menu_bar = true;
    hooks.on_open      = [&](reckless::app::AppWindow &opened) {
        window = &opened;
        sizes.SetWindow(&opened);
        follow();
        board.GrabKeyboardFocus();
    };
    hooks.on_frame = [&](double seconds) {
        sizes.Frame(seconds);
        view.Show(controller);
        follow();
        root.Frame();
    };
    hooks.on_end = [&](std::optional<std::pair<int, int>> position) {
        static_cast<void>(position);
        // The size in force kept, or the last one where the window was dragged to another.
        controller.SetWindowPercent(sizes.InForce());
        keeper.Changed(controller.settings());
        sizes.SetWindow(nullptr);
        window = nullptr;
    };
    return reckless::app::RunApp(*platform, root, {MainView::kMinimumWidth, MainView::kMinimumHeight}, options,
                                 "Reckless Sudoku", sudoku::kVersion, &reckless::window::CreateChildWindow, hooks);
}
