#pragma once
/// @file puzzle_file.h Puzzles in files. A .sud file is 81 bytes, one per square row by row from the top left, 0 for an
/// empty square. Any other file is text: the 81 squares row by row, a digit or '.' ('0' too) for an empty square,
/// whatever else it holds - line breaks, spaces - ignored; a .sdk file is nine lines of nine.

#include "solver/grid.h"

#include <filesystem>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace sudoku::puzzle_file
{

/** A file that is not a puzzle, or could not be read or written; what() says which, in words. */
class Error : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * @param path The file.
 * @return Its 81 values, 0 for an empty square.
 * @throws Error The file could not be read, or is not a puzzle.
 */
Values Read(const std::filesystem::path &path);

/**
 * Writes a puzzle: bytes for a .sud file (any case), nine lines of nine with CR LF line ends for any other.
 * @param path The file.
 * @param values 81 values, 0 for an empty square.
 * @throws Error The file could not be written.
 */
void Write(const std::filesystem::path &path, std::span<const int> values);

/**
 * @param text A puzzle as text.
 * @return Its 81 values, 0 for an empty square.
 * @throws Error The text does not hold exactly 81 squares.
 */
Values Parse(std::string_view text);

/**
 * @param values 81 values, 0 for an empty square.
 * @return The puzzle as nine lines of nine, '.' for an empty square, each line ending CR LF.
 */
std::string Format(std::span<const int> values);

} // namespace sudoku::puzzle_file
