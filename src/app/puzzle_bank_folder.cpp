#include "app/puzzle_bank_folder.h"

#include "solver/puzzle_bank.h"

#include <array>
#include <cstdint>
#include <string>
#include <system_error>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace sudoku::app
{

std::filesystem::path ExecutableFolder()
{
#if defined(_WIN32)
    std::wstring path(MAX_PATH, L'\0');
    while (true)
    {
        const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
        if (length == 0)
        {
            return {};
        }
        if (length < path.size())
        {
            path.resize(length);
            return std::filesystem::path(path).parent_path();
        }
        path.resize(path.size() * 2);
    }
#elif defined(__APPLE__)
    std::uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string path(size, '\0');
    if (_NSGetExecutablePath(path.data(), &size) != 0)
    {
        return {};
    }
    path.resize(path.find('\0') == std::string::npos ? path.size() : path.find('\0'));
    std::error_code             error;
    const std::filesystem::path real = std::filesystem::canonical(path, error);
    return error ? std::filesystem::path(path).parent_path() : real.parent_path();
#else
    std::error_code             error;
    const std::filesystem::path path = std::filesystem::read_symlink("/proc/self/exe", error);
    return error ? std::filesystem::path() : path.parent_path();
#endif
}

std::filesystem::path PuzzleBankBeside(const std::filesystem::path &executable_folder)
{
    if (executable_folder.empty())
    {
        return {};
    }
    const std::array<std::filesystem::path, 3> places{
        executable_folder / "puzzles",
        executable_folder.parent_path() / "Resources" / "puzzles",
        executable_folder.parent_path() / "share" / "reckless-sudoku" / "puzzles",
    };
    for (const std::filesystem::path &place : places)
    {
        if (puzzle_bank::IsBank(place))
        {
            return place;
        }
    }
    return {};
}

} // namespace sudoku::app
