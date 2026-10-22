#pragma once
/// @file controller.h The window's state, written against no toolkit: the game, the square chosen, the step lit, the
/// pencil marks, the puzzle's file, and what the window says - where the puzzle stands and what was last done.

#include "app/board_picture.h"
#include "solver/game.h"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>

namespace sudoku::app
{

/** The person's choices that outlive a launch (settings_file.h). */
struct Settings
{
    bool shows_pencil_marks{true}; ///< Whether empty squares show the digits they may take.
    int  window_percent{100};      ///< The window's size from the View menu, 50 to 150; 100 by default.

    /**
     * @param other Other settings.
     * @return Whether the two are the same.
     */
    bool operator==(const Settings &other) const = default;
};

/**
 * The window's state: what the board and the panel show, and what each of the person's acts does. Every act says what
 * it did in a message - a clue's step, why a solve stopped short, a clash - which stands until the next act.
 */
class Controller
{
  public:
    /** @param settings The choices kept from the last launch. */
    explicit Controller(Settings settings = {});

    /** @return The game. */
    const Game &game() const noexcept
    {
        return game_;
    }

    /** @return The choices kept between launches. */
    const Settings &settings() const noexcept
    {
        return settings_;
    }

    /** @return The square chosen, which the keys fill. */
    int selected() const noexcept
    {
        return selected_;
    }

    /** @return The step lit on the board, an index into the game's steps; none for none. */
    std::optional<std::size_t> shown_step() const noexcept
    {
        return shown_step_;
    }

    /** @return The puzzle's file name, as the title shows it; empty for a puzzle from no file. */
    const std::string &file_name() const noexcept
    {
        return file_name_;
    }

    /**
     * @param cell A square to choose.
     */
    void Select(int cell);

    /**
     * The square chosen moved by rows and columns, held to the board.
     * @param rows Rows down; negative for up.
     * @param columns Columns right; negative for left.
     */
    void MoveSelection(int rows, int columns);

    /**
     * A digit entered in a square as a given, or the square cleared; the board rebuilt from the givens.
     * @param cell The square.
     * @param digit 1 to 9; 0 to clear.
     */
    void Enter(int cell, int digit);

    /** Steps until one places a value. */
    void Clue();

    /** Steps until the board is complete or nothing more can be found. */
    void Solve();

    /** The puzzle's solutions counted, and the count said. */
    void Check();

    /** The last change taken back. */
    void Undo();

    /** Every square emptied, and the file forgotten. */
    void Clear();

    /**
     * A puzzle opened from a file (puzzle_file.h); a file that is not one leaves the board as it was and says why.
     * @param path The file.
     * @return Whether it was opened.
     */
    bool Open(const std::filesystem::path &path);

    /**
     * The givens saved to a file (puzzle_file.h): bytes for a .sud file, text for any other.
     * @param path The file.
     * @return Whether it was saved.
     */
    bool Save(const std::filesystem::path &path);

    /**
     * A step lit on the board, as the list chooses it.
     * @param index An index into the game's steps; none for the last.
     */
    void ShowStep(std::optional<std::size_t> index);

    /** @param shows Whether empty squares show their pencil marks. */
    void SetPencilMarks(bool shows);

    /** @param percent The window's size from the View menu, kept for the next launch; 0 for none (dragged). */
    void SetWindowPercent(int percent);

    /** @return Whether a clue or a solve can find anything: no clash, no contradiction, not solved. */
    bool CanRun() const;

    /** @return Where the puzzle stands, in a word or two: "Solved", "38 of 81", "Givens clash". */
    std::string Headline() const;

    /** @return What was last done, or where the puzzle stands when nothing has been said since the last change. */
    std::string Message() const;

    /** @return What the board shows now. */
    BoardPicture Picture() const;

  private:
    /** @return Why the solver stopped short: a contradiction, or the puzzle's solutions by search. */
    std::string WhyStuck() const;

    /** @return Where the puzzle stands, in a sentence. */
    std::string Standing() const;

    /** After an act: the last step lit, and the message set. @param message What to say; none for where it stands. */
    void Said(std::optional<std::string> message);

    Game                       game_;         ///< The game.
    Settings                   settings_;     ///< The choices kept between launches.
    int                        selected_{40}; ///< The square chosen.
    std::optional<std::size_t> shown_step_;   ///< The step lit; none for none.
    std::optional<std::string> message_;      ///< What was last done; none for where the puzzle stands.
    std::string                file_name_;    ///< The puzzle's file name; empty for none.
};

} // namespace sudoku::app
