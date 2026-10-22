#pragma once
/// @file puzzle_bank_folder.h Where the puzzle bank lies beside the app, as the build and the installers lay it out:
/// puzzles/ beside the executable (Windows, the Linux archive, a build tree), in the bundle's Resources (macOS), and in
/// share/reckless-sudoku beside the executable's bin folder (the Debian package).

#include <filesystem>

namespace sudoku::app
{

/** @return The folder of the running executable; empty where the system does not say. */
std::filesystem::path ExecutableFolder();

/**
 * @param executable_folder The executable's folder.
 * @return The first of the places the bank is laid out in, relative to it, that holds a whole bank
 * (puzzle_bank::IsBank); empty for none.
 */
std::filesystem::path PuzzleBankBeside(const std::filesystem::path &executable_folder);

} // namespace sudoku::app
