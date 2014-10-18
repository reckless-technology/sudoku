using System;
using System.Collections.Generic;
namespace Sudoku
{
    class Solver : ISudokuSolver
    {
        private Board board;
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="size">length of a sector; 3 for normal Sudoku</param>
        public Solver(int size)
        {
            board = new Board(size);
        }
        /// <summary>
        /// Tri-state for strategy results: used to determine if further iteration is warranted
        /// </summary>
        private enum StrategyResult
        {
            NoProgress,
            ProgressButNoMove,
            MoveFound
        }
        /// <summary>
        /// Used in the naked sibling and hidden sibling strategies
        /// </summary>
        private enum SiblingType
        {
            Naked,
            Hidden
        }

        /// <summary>
        /// Get and place values in the board.
        /// An assigned value of 0 will clear the square (i.e. set to default all candidates)
        /// </summary>
        /// <returns>square value if defined; 0 if ambiguous or no value</returns>
        public int this[int x, int y]
        {
            get
            {
                if (board[x, y].hasUniqueValue)
                {
                    return board[x, y].value;
                }
                else
                {
                    return 0;
                }
            }
            set
            {
                if (value != 0)
                {
                    board[x, y].Assign(value);
                }
                else
                {
                    board.ClearSquare(x, y);
                }
            }
        }

        public bool hasBeenSolved
        {
            get
            {
                return board.hasBeenSolved;
            }
        }
        /// <summary>
        /// Get the next move
        /// </summary>
        /// <param name="move">out reference to struct containing the move, if found</param>
        /// <returns>true if a move was found</returns>
        public bool GetNextMove(ref Move move)
        {
            bool canDoAnotherIteration = true;
            // Iterate until either a move is found or no further progress is made
            while (canDoAnotherIteration)
            {
                canDoAnotherIteration = false;
                switch (NakedSiblingStrategy(ref move))
                {
                    case StrategyResult.MoveFound:
                        return true;
                    case StrategyResult.ProgressButNoMove:
                        canDoAnotherIteration = true;
                        break;
                }
                switch (HiddenSiblingStrategy(ref move))
                {
                    case StrategyResult.MoveFound:
                        return true;
                    case StrategyResult.ProgressButNoMove:
                        canDoAnotherIteration = true;
                        break;
                }
                switch (LineSectorIntersectionStrategy(ref move))
                {
                    case StrategyResult.MoveFound:
                        return true;
                    case StrategyResult.ProgressButNoMove:
                        canDoAnotherIteration = true;
                        break;
                }
            }
            return false;
        }

        public bool Solve()
        {
            Move move = new Move();
            while (GetNextMove(ref move))
            {
            }
            return hasBeenSolved;
        }

        private static void PlaceMove(ref Move move, Square location, int candidate)
        {
            move.x = location.x;
            move.y = location.y;
            move.value = candidate;
            location.Assign(candidate);
        }

        /// <summary>
        /// Naked sibling strategy:
        /// 
        /// Consider a set A of m squares: typically A is a row, column or sector
        /// 
        /// If there exists a subset B of n squares (1 <= n < m), each of which contains PRECISELY
        /// ALL members of the set N of n candidate values, then B forms a closed subset of A.
        /// 
        /// In this case, the values N must be placed into the squares of B.
        /// 
        /// It is not yet determined into which square each distinct value should be
        /// placed. However candidate values which are in the set N may be 
        /// safely eliminated from squares which are members of A but not of B. 
        /// This may result in further progress becoming possible.
        /// 
        /// Depending upon the value of n, this strategy is known as  
        /// naked twin (2), naked triplet (3), ... naked octuplet(8)
        /// 
        /// The special case where n = 1 is equivalent
        /// to the determined value elimination strategy.
        /// 
        /// This strategy is NP-hard: brute force search is required
        /// </summary>
        private StrategyResult NakedSiblingStrategy(ref Move move)
        {
            return NakedHiddenSiblingStrategy(ref move, SiblingType.Naked);
        }

        /// <summary>
        /// Hidden sibling strategy:
        /// 
        /// Consider a set A of m squares.
        /// 
        /// If there exists a subset B of n squares (1 <= n < m), each of which contains 
        /// ANY member of the set N of n candidate values, then B forms an open subset of A.
        /// 
        /// The squares of B may also contain other candidates in addition to any member of N.
        /// 
        /// In this case, the values N will ultimately be placed into the squares of B. 
        /// 
        /// It is not yet determined into which square each distinct value should be
        /// placed. However candidate values which are not in the set N may be safely removed
        /// from the members of B.
        /// 
        /// This may result in further progress becoming possible.
        /// 
        /// This technique is complementary to the naked sibling strategy. The principal
        /// difference is that naked sibling strategy removes candidates from squares which are
        /// not members of the subset; hidden sibling strategy removes candidates from squares
        /// which are members of the subset. 
        /// 
        /// Depending upon the value of n, this strategy is known as 
        /// hidden twin (2), hidden triplet (3), ... hidden octuplet(8)
        /// 
        /// The special case where n = 1 is equivalent
        /// to the single value placement strategy.
        /// 
        /// This strategy is NP-hard: brute force search is required
        /// </summary>
        private StrategyResult HiddenSiblingStrategy(ref Move move)
        {
            return NakedHiddenSiblingStrategy(ref move, SiblingType.Hidden);
        }

        /// <summary>
        /// Generic implementation of both hidden and naked sibling strategies
        /// </summary>
        private StrategyResult NakedHiddenSiblingStrategy(ref Move move, SiblingType siblingType)
        {
            List<Square> squares;
            bool hasProgressBeenMade = false;
            bool canDoAnotherIteration = true;
            // Iterate until move found or no further progress is possible
            while (canDoAnotherIteration)
            {
                canDoAnotherIteration = false;
                // Iterate through all subset sizes
                for (int n = 1; n != board.boardLength; n++)
                {
                    // Process the columns
                    for (int x = 0; x != board.boardLength; x++)
                    {
                        squares = board.GetColumn(x);
                        switch (ProcessSiblings(n, squares, siblingType, ref move))
                        {
                            case StrategyResult.MoveFound:
                                return StrategyResult.MoveFound;
                            case StrategyResult.ProgressButNoMove:
                                hasProgressBeenMade = true;
                                canDoAnotherIteration = true;
                                break;
                        }
                    }
                    // Process the rows
                    for (int y = 0; y != board.boardLength; y++)
                    {
                        squares = board.GetRow(y);
                        switch (ProcessSiblings(n, squares, siblingType, ref move))
                        {
                            case StrategyResult.MoveFound:
                                return StrategyResult.MoveFound;
                            case StrategyResult.ProgressButNoMove:
                                hasProgressBeenMade = true;
                                canDoAnotherIteration = true;
                                break;
                        }
                    }
                    // Process the sectors
                    for (int x = 0; x != board.boardLength; x += board.sectorLength)
                    {
                        for (int y = 0; y != board.boardLength; y += board.sectorLength)
                        {
                            squares = board.GetSector(x, y);
                            switch (ProcessSiblings(n, squares, siblingType, ref move))
                            {
                                case StrategyResult.MoveFound:
                                    return StrategyResult.MoveFound;
                                case StrategyResult.ProgressButNoMove:
                                    hasProgressBeenMade = true;
                                    canDoAnotherIteration = true;
                                    break;
                            }
                        }
                    }
                }
            }
            if (hasProgressBeenMade)
            {
                return StrategyResult.ProgressButNoMove;
            }
            return StrategyResult.NoProgress;
        }

        /// <summary>
        /// Find naked or hidden siblings within a set and eliminate candidate values accodingly.
        /// </summary>
        private StrategyResult ProcessSiblings(int howManySiblings, List<Square> squares, SiblingType type, ref Move move)
        {
            bool hasProgressBeenMade = false;
            // Get all possible permutations of (howManySiblings) candidate values
            // as a List of bitsets
            List<uint> bitsets = GetBitsetGroup(howManySiblings, board.boardLength);
            List<Square> subset = new List<Square>();
            bool canDoAnotherIteration = true;
            while (canDoAnotherIteration)
            {
                canDoAnotherIteration = false;
                // Check each of the possible subsets of size howManySiblings
                foreach (uint bitset in bitsets)
                {
                    subset.Clear();
                    foreach (Square s in squares)
                    {
                        switch (type)
                        {
                            case SiblingType.Hidden:
                                if (s.HasAnyOf(bitset))
                                {
                                    subset.Add(s);
                                }
                                break;
                            case SiblingType.Naked:
                                if (s.HasOnly(bitset))
                                {
                                    subset.Add(s);
                                }
                                break;
                        }
                    }
                    if (subset.Count == howManySiblings)
                    {
                        // This is a suitable subset for processing, since the size of the
                        // subset equals the number of candidates under consideration.
                        foreach (Square s in squares)
                        {
                            switch (type)
                            {
                                case SiblingType.Hidden:
                                    // Remove values which are not in the bitset
                                    // from squares which are members of the subset
                                    if (subset.Contains(s))
                                    {
                                        if (s.AndMask(bitset))
                                        {
                                            hasProgressBeenMade = true;
                                            canDoAnotherIteration = true;
                                            if (s.hasUniqueValue)
                                            {
                                                PlaceMove(ref move, s, s.value);
                                                return StrategyResult.MoveFound;
                                            }
                                        }
                                    }
                                    break;

                                case SiblingType.Naked:
                                    // Remove values which are in the bitset from 
                                    // squares which are not members of the subset
                                    if (!subset.Contains(s))
                                    {
                                        if (s.AndMask(~bitset))
                                        {
                                            hasProgressBeenMade = true;
                                            canDoAnotherIteration = true;
                                            if (s.hasUniqueValue)
                                            {
                                                PlaceMove(ref move, s, s.value);
                                                return StrategyResult.MoveFound;
                                            }
                                        }
                                    }
                                    break;
                            }
                        }
                    }
                }
            }
            if (hasProgressBeenMade)
            {
                return StrategyResult.ProgressButNoMove;
            }
            return StrategyResult.NoProgress;
        }

        /// <summary>
        /// Line / Sector intersection strategy:
        /// 
        /// According to the rules of Sudoku, each value must occur precisely once
        /// per row, column and sector.
        /// 
        /// This strategy exploits the intersection between a sector of the Sudoku
        /// board, and a line (a line is either a row or a column).
        /// 
        /// Consider a candidate value v, a sector S and a line L (L must intersect S).
        /// 
        /// If all squares within S which contain v are also members of L, then v
        /// can be safely eliminated from squares which are members of L
        /// but not of S, since both S and L must contain v precisely once.
        /// 
        /// Conversely, if all squares within L which contain v are also members of S, then v
        /// can be safely eliminated from squares which are members of S
        /// but not of L, as both L and S must contain v precisely once.
        /// 
        /// This may result in further progress becoming possible.
        /// </summary>
        private StrategyResult LineSectorIntersectionStrategy(ref Move move)
        {
            bool hasProgressBeenMade = false;
            bool canDoAnotherIteration = true;
            List<Square> sector = null;
            List<Square> line = null;
            while (canDoAnotherIteration)
            {
                canDoAnotherIteration = false;
                // Process row / sector interactions
                for (int y = 0; y != board.boardLength; y++)
                {
                    line = board.GetRow(y);
                    for (int x = 0; x != board.boardLength; x += board.sectorLength)
                    {
                        sector = board.GetSector(x, y);
                        switch (ProcessLineSector(sector, line, ref move))
                        {
                            case StrategyResult.MoveFound:
                                return StrategyResult.MoveFound;
                            case StrategyResult.ProgressButNoMove:
                                hasProgressBeenMade = true;
                                canDoAnotherIteration = true;
                                break;
                        }
                    }
                }
                // Process column / sector intersections
                for (int x = 0; x != board.boardLength; x++)
                {
                    line = board.GetColumn(x);
                    for (int y = 0; y != board.boardLength; y += board.sectorLength)
                    {
                        sector = board.GetSector(x, y);
                        switch (ProcessLineSector(sector, line, ref move))
                        {
                            case StrategyResult.MoveFound:
                                return StrategyResult.MoveFound;
                            case StrategyResult.ProgressButNoMove:
                                hasProgressBeenMade = true;
                                canDoAnotherIteration = true;
                                break;
                        }
                    }
                }
            }
            if (hasProgressBeenMade)
            {
                return StrategyResult.ProgressButNoMove;
            }
            return StrategyResult.NoProgress;
        }

        private StrategyResult ProcessLineSector(List<Square> sector, List<Square> line, ref Move move)
        {
            bool hasProgressBeenMade = false;
            bool canDoAnotherIteration = true;
            List<Square> squaresInSectorWithCandidate = null;
            List<Square> squaresInLineWithCandidate = null;
            while (canDoAnotherIteration)
            {
                canDoAnotherIteration = false;
                for (int candidate = 1; candidate <= board.boardLength; candidate++)
                {
                    squaresInSectorWithCandidate = Board.FindCandidate(candidate, sector);
                    squaresInLineWithCandidate = Board.FindCandidate(candidate, line);
                    if (squaresInSectorWithCandidate.TrueForAll(delegate(Square s) { return line.Contains(s); }))
                    {
                        // Eliminate candidate from squares which are members of line
                        // but not of sector
                        foreach (Square s in line)
                        {
                            if (!sector.Contains(s))
                            {
                                if (s.Eliminate(candidate))
                                {
                                    hasProgressBeenMade = true;
                                    canDoAnotherIteration = true;
                                    if (s.hasUniqueValue)
                                    {
                                        move.x = s.x;
                                        move.y = s.y;
                                        move.value = s.value;
                                        return StrategyResult.MoveFound;
                                    }
                                }
                            }
                        }
                    }
                    if (squaresInLineWithCandidate.TrueForAll(delegate(Square s) { return sector.Contains(s); }))
                    {
                        // Eliminate candidate from squares which are members of sector
                        // but not of line
                        foreach (Square s in sector)
                        {
                            if (!line.Contains(s))
                            {
                                if (s.Eliminate(candidate))
                                {
                                    hasProgressBeenMade = true;
                                    canDoAnotherIteration = true;
                                    if (s.hasUniqueValue)
                                    {
                                        move.x = s.x;
                                        move.y = s.y;
                                        move.value = s.value;
                                        return StrategyResult.MoveFound;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (hasProgressBeenMade)
            {
                return StrategyResult.ProgressButNoMove;
            }
            return StrategyResult.NoProgress;
        }

        private static List<uint> GetBitsetGroup(int groupSize, int maxCandidateValue)
        {
            List<uint> bitsets = new List<uint>();
            uint maxBitsetValue = (1u << maxCandidateValue);
            for (uint u = 1u; u != maxBitsetValue; u++)
            {
                if (Square.Cardinality(u) == groupSize)
                {
                    bitsets.Add(u);
                }
            }
            return bitsets;
        }
    }
}



