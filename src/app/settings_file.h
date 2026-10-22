#pragma once
/// @file settings_file.h The person's choices kept between launches: `reckless-technology/sudoku/settings.json` in the
/// user's settings folder, a JSON object of whether the pencil marks show and the window's size.

#include "app/controller.h"

#include <filesystem>
#include <optional>
#include <utility>

namespace sudoku::app
{

/** @return `reckless-technology/sudoku/settings.json` in the user's settings folder; empty where there is none. */
std::filesystem::path DefaultSettingsFile();

/**
 * What `file` keeps: each choice read where it is there and makes sense, the default where it is missing, of another
 * kind or out of range (a window size outside 50 to 150); all the defaults where the file is missing or not JSON.
 * @param file The file.
 * @return The choices.
 */
Settings ReadSettings(const std::filesystem::path &file);

/**
 * The choices written to `file`, whole or not at all, its folder made first.
 * @param file The file; nothing is written where it is empty.
 * @param settings The choices.
 * @return Whether the file was written.
 */
bool WriteSettings(const std::filesystem::path &file, const Settings &settings);

/**
 * The choices kept in one file across a session: read once as the app starts, then told the controller's each frame,
 * writing the file whenever they differ from those it last saw - never for what it read.
 */
class SettingsKeeper
{
  public:
    /** @param file The file; empty to read the defaults and keep nothing (a launch's check). */
    explicit SettingsKeeper(std::filesystem::path file) : file_(std::move(file))
    {
    }

    /** @return What the file keeps (ReadSettings), which is then what it has seen. */
    Settings Read();

    /**
     * The choices now: written to the file when they differ from those last seen.
     * @param now The choices now.
     * @return Whether they differ - the first call always - so the menu's ticks can follow them.
     */
    bool Changed(const Settings &now);

    /** @return How many times the file has been written. */
    int writes() const noexcept
    {
        return writes_;
    }

  private:
    std::filesystem::path   file_;                ///< The file.
    std::optional<Settings> seen_;                ///< The choices last read or seen; none before either.
    bool                    has_reported_{false}; ///< Changed has been called.
    int                     writes_{0};           ///< The file's writes.
};

} // namespace sudoku::app
