#include "solver/digits.h"

namespace sudoku::digits
{

std::vector<int> Each(int set)
{
    std::vector<int> digits;
    for (int digit = 1; digit <= 9; digit++)
    {
        if (Contains(set, digit))
        {
            digits.push_back(digit);
        }
    }
    return digits;
}

std::string Format(int set)
{
    std::string text = "{";
    for (int digit : Each(set))
    {
        if (text.size() > 1)
        {
            text += ',';
        }
        text += std::to_string(digit);
    }
    return text + "}";
}

} // namespace sudoku::digits
