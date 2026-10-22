#include "solver/puzzle_bank.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <system_error>

namespace sudoku::puzzle_bank
{

std::string_view BandName(Band band)
{
    switch (band)
    {
    case Band::kEasy:
        return "Easy";
    case Band::kMedium:
        return "Medium";
    case Band::kHard:
        return "Hard";
    case Band::kDiabolical:
        return "Diabolical";
    }
    return "Easy";
}

std::string FileName(Band band)
{
    std::string name(BandName(band));
    std::transform(name.begin(), name.end(), name.begin(),
                   [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
    return name + ".txt";
}

bool IsBank(const std::filesystem::path &folder)
{
    return std::all_of(kBands.begin(), kBands.end(), [&](Band band) { return Count(folder, band) > 0; });
}

std::size_t Count(const std::filesystem::path &folder, Band band)
{
    std::error_code   error;
    const std::size_t size = static_cast<std::size_t>(std::filesystem::file_size(folder / FileName(band), error));
    return error || size % kRecordSize != 0 ? 0 : size / kRecordSize;
}

std::optional<Puzzle> Read(const std::filesystem::path &folder, Band band, std::size_t index)
{
    if (index >= Count(folder, band))
    {
        return std::nullopt;
    }
    std::ifstream file(folder / FileName(band), std::ios::binary);
    file.seekg(static_cast<std::streamoff>(index * kRecordSize));
    std::string record(kRecordSize, '\0');
    if (!file.read(record.data(), static_cast<std::streamsize>(kRecordSize)) || record.back() != '\n')
    {
        return std::nullopt;
    }
    record.pop_back();
    return Parse(record, band, index);
}

std::optional<Puzzle> Parse(std::string_view record, Band band, std::size_t index)
{
    const auto is_hex   = [](char c) { return std::isdigit(static_cast<unsigned char>(c)) || (c >= 'a' && c <= 'f'); };
    const auto is_digit = [](char c) { return c >= '0' && c <= '9'; };
    if (record.size() != kRecordSize - 1 || record[12] != ' ' || record[94] != ' ' ||
        !std::all_of(record.begin(), record.begin() + 12, is_hex) ||
        !std::all_of(record.begin() + 13, record.begin() + 94, is_digit))
    {
        return std::nullopt;
    }
    // The rating: right-aligned in four characters, " 1.2" or "10.4".
    const std::string rating(record.substr(95, 4));
    char             *end   = nullptr;
    const double      value = std::strtod(rating.c_str(), &end);
    if (end != rating.c_str() + rating.size())
    {
        return std::nullopt;
    }
    Puzzle puzzle{band, index, std::string(record.substr(0, 12)), {}, value};
    for (std::size_t square = 0; square < puzzle.givens.size(); ++square)
    {
        puzzle.givens[square] = record[13 + square] - '0';
    }
    return puzzle;
}

} // namespace sudoku::puzzle_bank
