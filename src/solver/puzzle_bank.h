#pragma once
/// @file puzzle_bank.h The Sudoku Exchange puzzle bank (puzzles/sudoku_exchange/README.md), as the app offers new
/// puzzles from it: a folder of four files, one for each band of ratings, each a run of 100-byte records - 12 hex
/// digits of an id, a space, the 81 squares row by row from the top left (0 for an empty one), a space, the rating
/// right-aligned in four characters, a line feed - so a record is read by its index alone, with one seek.

#include "solver/grid.h"

#include <array>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace sudoku::puzzle_bank
{

/** A band of the bank's ratings, as its files sort them. */
enum class Band
{
    kEasy,       ///< Rated below 1.5.
    kMedium,     ///< Below 2.5.
    kHard,       ///< Below 5.0.
    kDiabolical, ///< 5.0 or more.
};

/** Every band, the easiest first. */
inline constexpr std::array<Band, 4> kBands{Band::kEasy, Band::kMedium, Band::kHard, Band::kDiabolical};

/** A record's length, its line feed included. */
inline constexpr std::size_t kRecordSize = 100;

/**
 * @param band A band.
 * @return Its name as a person says it: "Easy", "Diabolical".
 */
std::string_view BandName(Band band);

/**
 * @param band A band.
 * @return Its file's name in the bank's folder: "easy.txt".
 */
std::string FileName(Band band);

/** A puzzle of the bank. */
struct Puzzle
{
    Band        band{Band::kEasy}; ///< Its band.
    std::size_t index{0};          ///< Its record's place in its band's file, from 0.
    std::string id;                ///< The 12 hex digits the bank names it by.
    Values      givens{};          ///< Its squares, 0 for an empty one.
    double      rating{0.0};       ///< Its rating.
};

/**
 * @param folder The bank's folder.
 * @return Whether it holds every band's file, each a whole number of records, none empty.
 */
bool IsBank(const std::filesystem::path &folder);

/**
 * @param folder The bank's folder.
 * @param band A band.
 * @return The records its file holds; 0 for a file that is not there or is not a whole number of records.
 */
std::size_t Count(const std::filesystem::path &folder, Band band);

/**
 * @param folder The bank's folder.
 * @param band A band.
 * @param index A record's place in the band's file, from 0.
 * @return The puzzle; none past the file's end, or for a record not in the bank's format.
 */
std::optional<Puzzle> Read(const std::filesystem::path &folder, Band band, std::size_t index);

/**
 * @param record A record without its line feed: 99 characters.
 * @param band The band it is read from.
 * @param index Its place in the band's file.
 * @return The puzzle; none for a record not in the bank's format.
 */
std::optional<Puzzle> Parse(std::string_view record, Band band, std::size_t index);

} // namespace sudoku::puzzle_bank
