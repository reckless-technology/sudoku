#pragma once
/// @file paths.h Paths to and from UTF-8, the encoding the toolkit's dialogs and the window's text use, the same on
/// every platform.

#include <filesystem>
#include <string>

namespace sudoku::app
{

/**
 * @param utf8 A path in UTF-8.
 * @return The path.
 */
inline std::filesystem::path PathFromUtf8(const std::string &utf8)
{
    return std::filesystem::path(std::u8string(utf8.begin(), utf8.end()));
}

/**
 * @param path A path.
 * @return It in UTF-8.
 */
inline std::string Utf8FromPath(const std::filesystem::path &path)
{
    const std::u8string text = path.u8string();
    return std::string(text.begin(), text.end());
}

} // namespace sudoku::app
