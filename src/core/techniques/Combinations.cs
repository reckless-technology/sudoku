namespace Sudoku.Techniques;

/// <summary>The ways to choose some items from a list.</summary>
internal static class Combinations
{
    /// <param name="items">The items.</param>
    /// <param name="size">How many to choose.</param>
    /// <returns>Every choice of <paramref name="size"/> of them, each in the list's order.</returns>
    public static IEnumerable<T[]> Of<T>(IReadOnlyList<T> items, int size)
    {
        if (size > items.Count)
        {
            yield break;
        }
        var indices = Enumerable.Range(0, size).ToArray();
        while (true)
        {
            yield return indices.Select(index => items[index]).ToArray();
            int position = size - 1;
            while (position >= 0 && indices[position] == items.Count - size + position)
            {
                position--;
            }
            if (position < 0)
            {
                yield break;
            }
            indices[position]++;
            for (int next = position + 1; next < size; next++)
            {
                indices[next] = indices[next - 1] + 1;
            }
        }
    }
}
