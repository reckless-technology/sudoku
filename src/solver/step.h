#pragma once
/// @file step.h One deduction - the technique, what it saw, what it placed or struck - and the techniques the solver
/// knows, in the order it tries them.

#include <array>
#include <compare>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace sudoku
{

/** The techniques the solver knows, in the order it tries them: the simplest first. */
enum class Technique
{
    kNakedSingle,        ///< A square with one candidate left.
    kHiddenSingle,       ///< A digit with one place left in a unit.
    kNakedPair,          ///< Two squares of a unit with two candidates between them.
    kHiddenPair,         ///< Two digits with the same two places in a unit.
    kPointing,           ///< A digit whose places in a box lie on one line: the rest of the line loses it.
    kClaiming,           ///< A digit whose places on a line lie in one box: the rest of the box loses it.
    kNakedTriple,        ///< Three squares of a unit with three candidates between them.
    kHiddenTriple,       ///< Three digits with the same three places in a unit.
    kNakedQuad,          ///< Four squares of a unit with four candidates between them.
    kHiddenQuad,         ///< Four digits with the same four places in a unit.
    kXWing,              ///< A digit's places in two rows on the same two columns, or the other way round.
    kSwordfish,          ///< The X-wing on three lines.
    kJellyfish,          ///< The X-wing on four lines.
    kXYWing,             ///< A two-candidate square and two it sees, which between them rule a digit out.
    kXYZWing,            ///< The XY-wing round a three-candidate square.
    kSimpleColouring,    ///< One digit's chains of strong links coloured in two, one colour true.
    kXYChain,            ///< A chain of two-candidate squares whose two ends cannot both lack a digit.
    kNishio,             ///< A digit that, placed in a square, leaves another unit no place for it.
    kForcingChain,       ///< A square whose every candidate leads, by singles, to the same conclusion.
    kContradictionChain, ///< A candidate that leads, by singles, to a contradiction.
};

/** Every technique, in the order the solver tries them. */
inline constexpr std::array<Technique, 20> kTechniques{
    Technique::kNakedSingle, Technique::kHiddenSingle, Technique::kNakedPair,    Technique::kHiddenPair,
    Technique::kPointing,    Technique::kClaiming,     Technique::kNakedTriple,  Technique::kHiddenTriple,
    Technique::kNakedQuad,   Technique::kHiddenQuad,   Technique::kXWing,        Technique::kSwordfish,
    Technique::kJellyfish,   Technique::kXYWing,       Technique::kXYZWing,      Technique::kSimpleColouring,
    Technique::kXYChain,     Technique::kNishio,       Technique::kForcingChain, Technique::kContradictionChain};

/**
 * @param technique A technique.
 * @return Its name as a person says it: "Hidden pair", "XY-wing".
 */
std::string_view TechniqueName(Technique technique);

/** A digit in a square: a value placed, or a candidate struck. */
struct Candidate
{
    int cell{0};  ///< The square.
    int digit{0}; ///< The digit.

    /** @return "g6=5". */
    std::string Text() const;

    /**
     * @param other Another.
     * @return Their order: by square, then by digit.
     */
    auto operator<=>(const Candidate &other) const = default;
};

/** One deduction: the technique, what it saw, and what it placed or struck. */
struct Step
{
    Technique              technique{Technique::kNakedSingle}; ///< The technique.
    std::string            description;                        ///< What it saw and did, in words.
    std::vector<Candidate> placements;                         ///< The values it placed.
    std::vector<Candidate> eliminations;                       ///< The candidates it struck.
    std::vector<int>       pattern; ///< The squares it reasoned from, for the board to light.

    /** @return Whether it placed a value. */
    bool IsPlacement() const noexcept
    {
        return !placements.empty();
    }
};

/**
 * @param eliminations Candidates struck.
 * @return Them in words, by digit, each digit's squares in order: "removes 3 from a6, i6 and 7 from a6";
 * "removes nothing" for none.
 */
std::string Removes(std::span<const Candidate> eliminations);

} // namespace sudoku
