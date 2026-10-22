namespace Sudoku;

/// <summary>The techniques the solver knows, in the order it tries them: the simplest first.</summary>
public enum Technique
{
    /// <summary>A square with one candidate left.</summary>
    NakedSingle,

    /// <summary>A digit with one place left in a unit.</summary>
    HiddenSingle,

    /// <summary>Two squares of a unit with the same two candidates between them.</summary>
    NakedPair,

    /// <summary>Two digits with the same two places in a unit.</summary>
    HiddenPair,

    /// <summary>A digit whose places in a box lie on one line, so the rest of the line loses it.</summary>
    Pointing,

    /// <summary>A digit whose places on a line lie in one box, so the rest of the box loses it.</summary>
    Claiming,

    /// <summary>Three squares of a unit with three candidates between them.</summary>
    NakedTriple,

    /// <summary>Three digits with the same three places in a unit.</summary>
    HiddenTriple,

    /// <summary>Four squares of a unit with four candidates between them.</summary>
    NakedQuad,

    /// <summary>Four digits with the same four places in a unit.</summary>
    HiddenQuad,

    /// <summary>A digit's places in two rows on the same two columns, or the other way round.</summary>
    XWing,

    /// <summary>The X-wing on three lines.</summary>
    Swordfish,

    /// <summary>The X-wing on four lines.</summary>
    Jellyfish,

    /// <summary>A two-candidate square and two others it sees, which between them rule a digit out.</summary>
    XYWing,

    /// <summary>The XY-wing round a three-candidate square.</summary>
    XYZWing,

    /// <summary>One digit's chains of strong links coloured in two, one colour true and the other false.</summary>
    SimpleColouring,

    /// <summary>A chain of two-candidate squares whose two ends cannot both lack a digit.</summary>
    XYChain,

    /// <summary>A digit that, placed in a square, leaves another unit no place for it.</summary>
    Nishio,

    /// <summary>A square whose every candidate leads, by singles, to the same conclusion.</summary>
    ForcingChain,

    /// <summary>A candidate that leads, by singles, to a contradiction.</summary>
    ContradictionChain,
}

/// <summary>A digit in a square: a value placed, or a candidate struck.</summary>
/// <param name="Cell">The square.</param>
/// <param name="Digit">The digit.</param>
public readonly record struct Candidate(int Cell, int Digit)
{
    /// <returns>"r4c7=5".</returns>
    public override string ToString() => $"{Geometry.Name(Cell)}={Digit}";
}

/// <summary>One deduction: the technique, what it saw, and what it placed or struck.</summary>
/// <param name="Technique">The technique.</param>
/// <param name="Description">What it saw and did, in words.</param>
/// <param name="Placements">The values it placed.</param>
/// <param name="Eliminations">The candidates it struck.</param>
/// <param name="Pattern">The squares it reasoned from, for the board to light.</param>
public sealed record Step(Technique Technique, string Description, IReadOnlyList<Candidate> Placements,
                          IReadOnlyList<Candidate> Eliminations, IReadOnlyList<int> Pattern)
{
    /// <summary>Whether it placed a value.</summary>
    public bool IsPlacement => Placements.Count > 0;

    /// <returns>The description.</returns>
    public override string ToString() => Description;
}

/// <summary>The techniques' names.</summary>
public static class TechniqueNames
{
    /// <param name="technique">A technique.</param>
    /// <returns>Its name as a person says it: "Hidden pair", "XY-wing".</returns>
    public static string Name(this Technique technique) => technique switch
    {
        Technique.NakedSingle => "Naked single",
        Technique.HiddenSingle => "Hidden single",
        Technique.NakedPair => "Naked pair",
        Technique.HiddenPair => "Hidden pair",
        Technique.Pointing => "Pointing",
        Technique.Claiming => "Claiming",
        Technique.NakedTriple => "Naked triple",
        Technique.HiddenTriple => "Hidden triple",
        Technique.NakedQuad => "Naked quad",
        Technique.HiddenQuad => "Hidden quad",
        Technique.XWing => "X-wing",
        Technique.Swordfish => "Swordfish",
        Technique.Jellyfish => "Jellyfish",
        Technique.XYWing => "XY-wing",
        Technique.XYZWing => "XYZ-wing",
        Technique.SimpleColouring => "Simple colouring",
        Technique.XYChain => "XY-chain",
        Technique.Nishio => "Nishio",
        Technique.ForcingChain => "Forcing chain",
        Technique.ContradictionChain => "Contradiction chain",
        _ => technique.ToString(),
    };

    /// <param name="eliminations">Candidates struck.</param>
    /// <returns>Them in words, by digit: "removes 3 from r4c1, r4c9 and 7 from r4c1".</returns>
    public static string Removes(IEnumerable<Candidate> eliminations)
    {
        var parts = eliminations.GroupBy(candidate => candidate.Digit)
                        .OrderBy(group => group.Key)
                        .Select(group => $"{group.Key} from {Geometry.Names(group.Select(c => c.Cell).Order())}")
                        .ToList();
        return parts.Count switch
        {
            0 => "removes nothing",
            1 => "removes " + parts[0],
            _ => "removes " + string.Join(", ", parts.Take(parts.Count - 1)) + " and " + parts[^1],
        };
    }
}
