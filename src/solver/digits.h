#pragma once
/// @file digits.h Sets of the digits 1 to 9 held as nine bits, the digit d at bit d - 1.

#include <bit>
#include <string>
#include <vector>

namespace sudoku::digits
{

inline constexpr int kAll = 0x1ff; ///< Every digit.

/**
 * @param digit A digit, 1 to 9.
 * @return The set holding that digit alone.
 */
constexpr int Bit(int digit) noexcept
{
    return 1 << (digit - 1);
}

/**
 * @param set A set of digits.
 * @return How many digits it holds.
 */
constexpr int Count(int set) noexcept
{
    return std::popcount(static_cast<unsigned>(set));
}

/**
 * @param set A set of digits.
 * @param digit A digit, 1 to 9.
 * @return Whether the set holds it.
 */
constexpr bool Contains(int set, int digit) noexcept
{
    return (set & Bit(digit)) != 0;
}

/**
 * @param set A set of digits, not empty.
 * @return Its smallest digit.
 */
constexpr int First(int set) noexcept
{
    return std::countr_zero(static_cast<unsigned>(set)) + 1;
}

/**
 * @param set A set of digits.
 * @return Its digits, smallest first.
 */
std::vector<int> Each(int set);

/**
 * @param set A set of digits.
 * @return The set as text: "{3,7}".
 */
std::string Format(int set);

} // namespace sudoku::digits
