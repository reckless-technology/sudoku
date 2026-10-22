#include "solver/step.h"

#include "solver/geometry.h"

#include <algorithm>

namespace sudoku
{

std::string_view TechniqueName(Technique technique)
{
    switch (technique)
    {
    case Technique::kNakedSingle:
        return "Naked single";
    case Technique::kHiddenSingle:
        return "Hidden single";
    case Technique::kNakedPair:
        return "Naked pair";
    case Technique::kHiddenPair:
        return "Hidden pair";
    case Technique::kPointing:
        return "Pointing";
    case Technique::kClaiming:
        return "Claiming";
    case Technique::kNakedTriple:
        return "Naked triple";
    case Technique::kHiddenTriple:
        return "Hidden triple";
    case Technique::kNakedQuad:
        return "Naked quad";
    case Technique::kHiddenQuad:
        return "Hidden quad";
    case Technique::kXWing:
        return "X-wing";
    case Technique::kSwordfish:
        return "Swordfish";
    case Technique::kJellyfish:
        return "Jellyfish";
    case Technique::kXYWing:
        return "XY-wing";
    case Technique::kXYZWing:
        return "XYZ-wing";
    case Technique::kSimpleColouring:
        return "Simple colouring";
    case Technique::kXYChain:
        return "XY-chain";
    case Technique::kNishio:
        return "Nishio";
    case Technique::kForcingChain:
        return "Forcing chain";
    case Technique::kContradictionChain:
        return "Contradiction chain";
    }
    return "Unknown technique";
}

std::string Candidate::Text() const
{
    return geometry::Name(cell) + "=" + std::to_string(digit);
}

std::string Removes(std::span<const Candidate> eliminations)
{
    std::vector<std::string> parts;
    for (int digit = 1; digit <= 9; digit++)
    {
        std::vector<int> cells;
        for (const Candidate &elimination : eliminations)
        {
            if (elimination.digit == digit)
            {
                cells.push_back(elimination.cell);
            }
        }
        if (!cells.empty())
        {
            std::sort(cells.begin(), cells.end());
            parts.push_back(std::to_string(digit) + " from " + geometry::Names(cells));
        }
    }
    if (parts.empty())
    {
        return "removes nothing";
    }
    std::string text = "removes " + parts.front();
    for (std::size_t i = 1; i < parts.size(); i++)
    {
        text += (i + 1 == parts.size() ? " and " : ", ") + parts[i];
    }
    return text;
}

} // namespace sudoku
