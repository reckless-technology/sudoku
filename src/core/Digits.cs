using System.Numerics;

namespace Sudoku;

/// <summary>Sets of the digits 1 to 9 held as nine bits, the digit d at bit d - 1.</summary>
public static class Digits
{
    /// <summary>Every digit.</summary>
    public const int All = 0x1FF;

    /// <param name="digit">A digit, 1 to 9.</param>
    /// <returns>The set holding that digit alone.</returns>
    public static int Bit(int digit) => 1 << (digit - 1);

    /// <param name="set">A set of digits.</param>
    /// <returns>How many digits it holds.</returns>
    public static int Count(int set) => BitOperations.PopCount((uint)set);

    /// <param name="set">A set of digits.</param>
    /// <param name="digit">A digit, 1 to 9.</param>
    /// <returns>Whether the set holds it.</returns>
    public static bool Contains(int set, int digit) => (set & Bit(digit)) != 0;

    /// <param name="set">A set of digits, not empty.</param>
    /// <returns>Its smallest digit.</returns>
    public static int First(int set) => BitOperations.TrailingZeroCount(set) + 1;

    /// <param name="set">A set of digits.</param>
    /// <returns>Its digits, smallest first.</returns>
    public static IEnumerable<int> Each(int set)
    {
        for (int digit = 1; digit <= 9; digit++)
        {
            if (Contains(set, digit))
            {
                yield return digit;
            }
        }
    }

    /// <param name="set">A set of digits.</param>
    /// <returns>The set as text: "{3,7}".</returns>
    public static string Format(int set) => "{" + string.Join(",", Each(set)) + "}";
}
