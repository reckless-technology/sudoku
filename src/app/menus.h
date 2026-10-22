#pragma once
/// @file menus.h The menu bar - File (with New puzzle's four bands), Edit, Puzzle, View and Help - each choice ticked
/// and enabled as the puzzle stands, and what each command does.

#include "app/controller.h"

#include "AppWindowSizes.h"
#include "MenuBar.h"

#include <functional>

namespace sudoku::app
{

using reckless::app::kWindowSizes;   ///< The View menu's sizes, the toolkit's (AppWindowSizes.h).
using reckless::app::WindowSizeMenu; ///< What the View menu shows of them.
using reckless::app::WindowSizes;    ///< The window held at them.

/** The menu's commands, as the bar reports them. */
enum Command : int
{
    kNoCommand      = 0,   ///< Nothing.
    kOpen           = 1,   ///< A puzzle opened from a file.
    kSave           = 2,   ///< The givens saved to a file.
    kQuit           = 3,   ///< The app ends.
    kUndo           = 4,   ///< The last change taken back.
    kClearBoard     = 5,   ///< Every square emptied.
    kClue           = 6,   ///< Steps until a value is placed.
    kSolve          = 7,   ///< Steps until the board is full or nothing more is found.
    kCheck          = 8,   ///< The puzzle's solutions counted.
    kPencilMarks    = 9,   ///< The pencil marks on or off.
    kLarger         = 10,  ///< The window a size larger.
    kSmaller        = 11,  ///< A size smaller.
    kAbout          = 12,  ///< The About card.
    kNewEasy        = 13,  ///< A new easy puzzle from the bank; kNewEasy + a band's place in kBands for the others.
    kNewMedium      = 14,  ///< A new medium puzzle.
    kNewHard        = 15,  ///< A new hard puzzle.
    kNewDiabolical  = 16,  ///< A new diabolical puzzle.
    kWindowSizeBase = 200, ///< kWindowSizeBase + percent: the window at that size.
};

/** What the bar shows: the choices and what can be done now. */
struct MenuState
{
    Settings       settings;               ///< The choices, ticked.
    bool           can_undo{false};        ///< Undo is enabled.
    bool           can_run{false};         ///< Clue and Solve are enabled.
    bool           has_puzzle_bank{false}; ///< New puzzle's choices are enabled.
    WindowSizeMenu window;                 ///< The window's sizes.

    /**
     * @param other Another state.
     * @return Whether the two show the same bar.
     */
    bool operator==(const MenuState &other) const = default;
};

/**
 * @param controller The window's state.
 * @param window The window's sizes as the View menu shows them.
 * @return The bar's state now.
 */
MenuState MenuStateOf(const Controller &controller, const WindowSizeMenu &window);

/**
 * The bar for `state`.
 * @param state What it shows.
 * @return The bar.
 */
reckless::app::MenuBar BuildMenuBar(const MenuState &state);

/** What the commands that reach past the puzzle do. */
struct CommandHandlers
{
    std::function<void()>            open;        ///< Asks for a file to open; may be empty.
    std::function<void()>            save;        ///< Asks for a file to save to; may be empty.
    std::function<void()>            quit;        ///< Ends the app; may be empty.
    std::function<void()>            about;       ///< Shows the About card; may be empty.
    std::function<void(int percent)> window_size; ///< Sizes the window; may be empty.
};

/**
 * Does what a command says.
 * @param command The command, from the bar or a shortcut.
 * @param controller The window's state.
 * @param handlers What the commands that reach past the puzzle do.
 * @param window The window's sizes, for Larger and Smaller.
 */
void RunCommand(int command, Controller &controller, const CommandHandlers &handlers, const WindowSizeMenu &window);

} // namespace sudoku::app
