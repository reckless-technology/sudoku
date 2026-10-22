#include "solver/puzzle_file.h"

#include <fstream>
#include <iterator>
#include <vector>

namespace sudoku::puzzle_file
{

namespace
{

/**
 * @param path A file.
 * @return Whether it is a .sud file, its name ending ".sud" in any case.
 */
bool IsBinary(const std::filesystem::path &path)
{
    const std::filesystem::path::string_type name = path.filename().native();
    if (name.size() < 4)
    {
        return false;
    }
    const char *extension = ".sud";
    for (std::size_t i = 0; i < 4; i++)
    {
        auto character = name[name.size() - 4 + i];
        if (character >= 'A' && character <= 'Z')
        {
            character = static_cast<decltype(character)>(character - 'A' + 'a');
        }
        if (character != static_cast<decltype(character)>(extension[i]))
        {
            return false;
        }
    }
    return true;
}

/**
 * @param bytes A .sud file's contents.
 * @return Its 81 values.
 * @throws Error The file is not 81 bytes of 0 to 9.
 */
Values FromBytes(const std::vector<char> &bytes)
{
    Values values{};
    bool   is_puzzle = bytes.size() == values.size();
    for (std::size_t i = 0; is_puzzle && i < bytes.size(); i++)
    {
        auto value = static_cast<unsigned char>(bytes[i]);
        is_puzzle  = value <= 9;
        values[i]  = value;
    }
    if (!is_puzzle)
    {
        throw Error("a .sud file is " + std::to_string(geometry::kCellCount) + " bytes of 0 to 9");
    }
    return values;
}

/**
 * @param path A file.
 * @return Its name, UTF-8, for a message.
 */
std::string DisplayName(const std::filesystem::path &path)
{
    std::u8string name = path.filename().u8string();
    return std::string(name.begin(), name.end());
}

} // namespace

Values Read(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        throw Error("could not read " + DisplayName(path));
    }
    std::vector<char> bytes{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    if (file.bad())
    {
        throw Error("could not read " + DisplayName(path));
    }
    return IsBinary(path) ? FromBytes(bytes) : Parse(std::string_view(bytes.data(), bytes.size()));
}

void Write(const std::filesystem::path &path, std::span<const int> values)
{
    std::string contents;
    if (IsBinary(path))
    {
        for (int value : values)
        {
            contents += static_cast<char>(value);
        }
    }
    else
    {
        contents = Format(values);
    }
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    file.close();
    if (!file)
    {
        throw Error("could not write " + DisplayName(path));
    }
}

Values Parse(std::string_view text)
{
    Values values{};
    int    count = 0;
    for (char character : text)
    {
        if (character != '.' && (character < '0' || character > '9'))
        {
            continue;
        }
        if (count < geometry::kCellCount)
        {
            values[static_cast<std::size_t>(count)] = character == '.' ? 0 : character - '0';
        }
        count++;
    }
    if (count != geometry::kCellCount)
    {
        throw Error("a puzzle has " + std::to_string(geometry::kCellCount) + " squares; this text has " +
                    std::to_string(count));
    }
    return values;
}

std::string Format(std::span<const int> values)
{
    std::string text;
    for (int row = 0; row < 9; row++)
    {
        for (int column = 0; column < 9; column++)
        {
            int value = values[static_cast<std::size_t>(geometry::Cell(row, column))];
            text += value == 0 ? '.' : static_cast<char>('0' + value);
        }
        text += "\r\n";
    }
    return text;
}

} // namespace sudoku::puzzle_file
