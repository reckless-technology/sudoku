#pragma once
/// @file techniques.h The techniques. Each looks at a board and returns the first deduction it finds as a Step - what
/// it saw, the values it places, the candidates it strikes, the squares it reasoned from and what it read there -
/// without changing the board; none when it finds nothing. Every step a technique returns places a value in an empty
/// square or strikes a candidate the board still has, and every reason it gives is a candidate or a value the board
/// holds.

#include "solver/grid.h"
#include "solver/step.h"

#include <optional>
#include <string>

namespace sudoku::techniques
{

/** The first empty square with one candidate left. */
std::optional<Step> NakedSingle(const Grid &grid);

/** The first digit with one place left in a unit, the units in Units' order. */
std::optional<Step> HiddenSingle(const Grid &grid);

/**
 * The first naked subset of `size` squares that strikes a candidate: `size` empty squares of a unit whose candidates
 * are `size` digits between them, each square holding any of them ({1,2}, {2,3}, {1,3} is a naked triple). The unit's
 * other squares lose those digits. A unit with no more than `size` empty squares is passed over.
 * @param grid The board.
 * @param size 2, 3 or 4.
 */
std::optional<Step> NakedSubset(const Grid &grid, int size);

/**
 * The first hidden subset of `size` digits that strikes a candidate: `size` digits whose places in a unit are the same
 * `size` squares, which lose every other candidate. A unit lacking no more than `size` digits is passed over.
 * @param grid The board.
 * @param size 2, 3 or 4.
 */
std::optional<Step> HiddenSubset(const Grid &grid, int size);

/** The first naked pair, then the first hidden pair. */
std::optional<Step> Pairs(const Grid &grid);

/** The first naked triple, then the first hidden triple. */
std::optional<Step> Triples(const Grid &grid);

/** The first naked quad, then the first hidden quad. */
std::optional<Step> Quads(const Grid &grid);

/** The first digit whose places in a box, two or more, lie on one row or column, that strikes it from the line. */
std::optional<Step> Pointing(const Grid &grid);

/** The first digit whose places on a row or column, two or more, lie in one box, that strikes it from the box. */
std::optional<Step> Claiming(const Grid &grid);

/** Pointing, then claiming. */
std::optional<Step> Intersections(const Grid &grid);

/** The first X-wing (a digit's places on two lines confined to two cross lines) that strikes a candidate. */
std::optional<Step> XWing(const Grid &grid);

/** The first swordfish (three lines) that strikes a candidate. */
std::optional<Step> Swordfish(const Grid &grid);

/** The first jellyfish (four lines) that strikes a candidate. */
std::optional<Step> Jellyfish(const Grid &grid);

/**
 * The first XY-wing that strikes a candidate: a pivot {x,y} seeing pincers {x,z} and {y,z}; a square seeing both
 * pincers loses z.
 */
std::optional<Step> XYWing(const Grid &grid);

/**
 * The first XYZ-wing that strikes a candidate: a pivot {x,y,z} seeing pincers {x,z} and {y,z}; a square seeing all
 * three loses z.
 */
std::optional<Step> XYZWing(const Grid &grid);

/**
 * The first simple colouring that strikes a candidate: a digit's strong links (units where it has exactly two places)
 * form chains coloured alternately; two squares of one colour in a unit make that colour false (a wrap), and a square
 * seeing both colours loses the digit (a trap).
 */
std::optional<Step> SimpleColouring(const Grid &grid);

/**
 * The first XY-chain, of three squares or more, that strikes a candidate: two-candidate squares, each seeing the next
 * and sharing a digit with it, whose ends cannot both lack a digit z; a square seeing both ends loses z. Breadth first
 * from each start, so the shortest chain is found.
 */
std::optional<Step> XYChain(const Grid &grid);

/**
 * The first Nishio: a digit placed in a square, and the digit's own hidden singles followed, leaves a unit with no
 * place for it - so the square loses the digit.
 */
std::optional<Step> Nishio(const Grid &grid);

/**
 * The first forcing chain: a square with two or three candidates, each tried and its singles followed with none ending
 * in a contradiction, every trial placing the same value in another square (a placement) or striking the same
 * candidates (eliminations).
 */
std::optional<Step> ForcingChain(const Grid &grid);

/** The first contradiction chain: a candidate whose singles, followed, end in a contradiction - so it is struck. */
std::optional<Step> ContradictionChain(const Grid &grid);

/**
 * Naked and hidden singles placed until none is left, or the board contradicts itself.
 * @param grid The board, changed.
 * @return The contradiction met (Grid::FindContradiction); none for none.
 */
std::optional<std::string> Propagate(Grid &grid);

} // namespace sudoku::techniques
