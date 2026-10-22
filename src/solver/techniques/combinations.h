#pragma once
/// @file combinations.h The ways to choose some items from a list, for the techniques that look at subsets.

#include <cstddef>
#include <vector>

namespace sudoku::techniques
{

/**
 * @tparam T The items' type.
 * @param items The items.
 * @param size How many to choose.
 * @return Every choice of `size` of them, each in the list's order, the choices in order of the items' positions:
 * {1,2}, {1,3}, {2,3} from {1,2,3}; none when there are fewer than `size` items.
 */
template <typename T> std::vector<std::vector<T>> Combinations(const std::vector<T> &items, int size)
{
    std::vector<std::vector<T>> choices;
    if (size < 0 || static_cast<std::size_t>(size) > items.size())
    {
        return choices;
    }
    const auto               count = static_cast<std::size_t>(size);
    std::vector<std::size_t> indices(count);
    for (std::size_t i = 0; i < count; i++)
    {
        indices[i] = i;
    }
    while (true)
    {
        std::vector<T> choice;
        choice.reserve(count);
        for (std::size_t index : indices)
        {
            choice.push_back(items[index]);
        }
        choices.push_back(std::move(choice));
        // The rightmost index that can still move right, moved, and those after it packed behind it.
        std::size_t position = count;
        while (position > 0 && indices[position - 1] == items.size() - count + position - 1)
        {
            position--;
        }
        if (position == 0)
        {
            return choices;
        }
        indices[position - 1]++;
        for (std::size_t next = position; next < count; next++)
        {
            indices[next] = indices[next - 1] + 1;
        }
    }
}

} // namespace sudoku::techniques
