#include "app/menus.h"

#include <string>
#include <utility>
#include <vector>

namespace sudoku::app
{

namespace
{

using reckless::app::MenuEntry;
using reckless::app::Shortcut;
using reckless::app::ShortcutKey;

/**
 * A menu item.
 * @param title Its text.
 * @param command What it reports.
 * @param is_checked Whether it is ticked.
 * @param shortcut Its key; none for none.
 * @param is_enabled Whether it can be chosen.
 * @return The item.
 */
MenuEntry Item(std::string title, int command, bool is_checked = false, Shortcut shortcut = {}, bool is_enabled = true)
{
    MenuEntry entry = MenuEntry::Item(std::move(title), command, shortcut);
    entry.checked   = is_checked;
    entry.enabled   = is_enabled;
    return entry;
}

} // namespace

MenuState MenuStateOf(const Controller &controller, const WindowSizeMenu &window)
{
    return MenuState{controller.settings(), controller.game().CanUndo(), controller.CanRun(),
                     controller.HasPuzzleBank(), window};
}

reckless::app::MenuBar BuildMenuBar(const MenuState &state)
{
    // New puzzle: a band each, Ctrl+1 to Ctrl+4, the easiest first.
    std::vector<MenuEntry> bands;
    for (std::size_t index = 0; index < puzzle_bank::kBands.size(); ++index)
    {
        bands.push_back(Item(std::string(puzzle_bank::BandName(puzzle_bank::kBands[index])),
                             kNewEasy + static_cast<int>(index), false,
                             Shortcut::Primary(static_cast<char>('1' + index)), state.has_puzzle_bank));
    }
    std::vector<MenuEntry> file{MenuEntry::Menu("New puzzle", std::move(bands)),
                                Item("Open...", kOpen, false, Shortcut::Primary('o')),
                                Item("Save...", kSave, false, Shortcut::Primary('s')), MenuEntry::Separator(),
                                Item("Quit", kQuit, false, Shortcut::Primary('q'))};
    std::vector<MenuEntry> edit{Item("Undo", kUndo, false, Shortcut::Primary('z'), state.can_undo),
                                Item("Clear board", kClearBoard)};
    std::vector<MenuEntry> puzzle{Item("Clue", kClue, false, Shortcut::Plain(ShortcutKey::kF5), state.can_run),
                                  Item("Solve", kSolve, false, Shortcut::Plain(ShortcutKey::kF6), state.can_run),
                                  Item("Check solutions", kCheck, false, Shortcut::Plain(ShortcutKey::kF7))};
    std::vector<MenuEntry> view{
        Item("Pencil marks", kPencilMarks, state.settings.shows_pencil_marks, Shortcut::Primary('p')),
        MenuEntry::Separator()};
    for (std::size_t index = 0; index < kWindowSizes.size(); ++index)
    {
        const int percent = kWindowSizes[index];
        view.push_back(Item(std::to_string(percent) + " %", kWindowSizeBase + percent, state.window.in_force == percent,
                            {}, state.window.fits[index]));
    }
    MenuEntry larger       = Item("Larger", kLarger, false, Shortcut::Primary('='), state.window.larger != 0);
    larger.other_shortcuts = {Shortcut::Primary('=', true), Shortcut::Primary('+')};
    view.push_back(MenuEntry::Separator());
    view.push_back(std::move(larger));
    view.push_back(Item("Smaller", kSmaller, false, Shortcut::Primary('-'), state.window.smaller != 0));

    reckless::app::MenuBar bar{{MenuEntry::Menu("File", std::move(file)), MenuEntry::Menu("Edit", std::move(edit)),
                                MenuEntry::Menu("Puzzle", std::move(puzzle)), MenuEntry::Menu("View", std::move(view)),
                                MenuEntry::Menu("Help", {Item("About Reckless Sudoku", kAbout)})}};
    bar.about_id = kAbout; // the Mac app menu's About opens the app's own card
    return bar;
}

void RunCommand(int command, Controller &controller, const CommandHandlers &handlers, const WindowSizeMenu &window)
{
    const auto tell = [](const std::function<void()> &handler) {
        if (handler)
        {
            handler();
        }
    };
    const auto size_window = [&handlers](int percent) {
        if (percent != 0 && handlers.window_size)
        {
            handlers.window_size(percent);
        }
    };
    if (command > kWindowSizeBase && WindowSizes::IsOffered(command - kWindowSizeBase))
    {
        size_window(command - kWindowSizeBase);
        return;
    }
    switch (command)
    {
    case kNewEasy:
    case kNewMedium:
    case kNewHard:
    case kNewDiabolical:
        controller.NewPuzzle(puzzle_bank::kBands[static_cast<std::size_t>(command - kNewEasy)]);
        break;
    case kOpen:
        tell(handlers.open);
        break;
    case kSave:
        tell(handlers.save);
        break;
    case kQuit:
        tell(handlers.quit);
        break;
    case kUndo:
        controller.Undo();
        break;
    case kClearBoard:
        controller.Clear();
        break;
    case kClue:
        if (controller.CanRun())
        {
            controller.Clue();
        }
        break;
    case kSolve:
        if (controller.CanRun())
        {
            controller.Solve();
        }
        break;
    case kCheck:
        controller.Check();
        break;
    case kPencilMarks:
        controller.SetPencilMarks(!controller.settings().shows_pencil_marks);
        break;
    case kLarger:
        size_window(window.larger);
        break;
    case kSmaller:
        size_window(window.smaller);
        break;
    case kAbout:
        tell(handlers.about);
        break;
    default:
        break;
    }
}

} // namespace sudoku::app
