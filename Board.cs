using System;
using System.Collections.Generic;
namespace Sudoku
{
    class Board
    {
        private Square[,] theBoard;
        public readonly int sectorLength, boardLength;

        public Board(int size)
        {
            sectorLength = size;
            boardLength = sectorLength * sectorLength;
            theBoard = new Square[boardLength, boardLength];
            for (int x = 0; x != boardLength; x++)
            {
                for (int y = 0; y != boardLength; y++)
                {
                    theBoard[x, y] = new Square(boardLength, x, y);
                }
            }
        }
        /// <summary>
        /// Copy constructor: performs deep copy of each Square
        /// </summary>
        public Board(Board other)
        {
            sectorLength = other.sectorLength;
            boardLength = other.boardLength;
            theBoard = new Square[boardLength, boardLength];
            for (int x = 0; x != boardLength; x++)
            {
                for (int y = 0; y != boardLength; y++)
                {
                    theBoard[x, y] = new Square(other.theBoard[x, y]);
                }
            }
        }
        /// <summary>
        /// Allow direct readonly 2D access to the squares of the board
        /// </summary>
        public Square this[int x, int y]
        {
            get
            {
                return theBoard[x, y];
            }
        }

        public void ClearSquare(int x, int y)
        {
            theBoard[x, y] = new Square(boardLength, x, y);
        }

        public void AddOtherSquaresInSector(Square refSquare, List<Square> otherSquares)
        {
            int startX = refSquare.x - (refSquare.x % sectorLength);
            int startY = refSquare.y - (refSquare.y % sectorLength);
            for (int x = startX; x != startX + sectorLength; x++)
            {
                for (int y = startY; y != startY + sectorLength; y++)
                {
                    if (x != refSquare.x || y != refSquare.y)
                    {
                        otherSquares.Add(theBoard[x, y]);
                    }
                }
            }
        }

        public void AddOtherSquaresInRow(Square refSquare, List<Square> otherSquares)
        {
            for (int x = 0; x != boardLength; x++)
            {
                if (x != refSquare.x)
                {
                    otherSquares.Add(theBoard[x, refSquare.y]);
                }
            }
        }

        public void AddOtherSquaresInColumn(Square refSquare, List<Square> otherSquares)
        {
            for (int y = 0; y != boardLength; y++)
            {
                if (y != refSquare.y)
                {
                    otherSquares.Add(theBoard[refSquare.x, y]);
                }
            }
        }

        public List<Square> GetRow(int y)
        {
            List<Square> squares = new List<Square>();
            squares.Add(theBoard[0, y]);
            AddOtherSquaresInRow(squares[0], squares);
            return squares;
        }

        public List<Square> GetColumn(int x)
        {
            List<Square> squares = new List<Square>();
            squares.Add(theBoard[x, 0]);
            AddOtherSquaresInColumn(squares[0], squares);
            return squares;
        }

        public List<Square> GetSector(int x, int y)
        {
            x = x - (x % sectorLength);
            y = y - (y % sectorLength);
            List<Square> squares = new List<Square>();
            squares.Add(theBoard[x, y]);
            AddOtherSquaresInSector(squares[0], squares);
            return squares;
        }

        /// <summary>
        /// A set is complete if each value appears precisely once.
        /// </summary>
        public static bool SetIsComplete(List<Square> squares)
        {
            List<int> values = new List<int>();
            foreach (Square s in squares)
            {
                if (!s.hasUniqueValue)
                {
                    return false;
                }
                values.Add(s.value);
            }
            values.Sort();
            for (int i = 0; i != values.Count; i++)
            {
                if (values[i] != i + 1)
                {
                    return false;   // Missing or duplicate value
                }
            }
            return true;
        }

        /// <summary>
        /// Find all the squares in a set which contain a target candidate value.
        /// </summary>
        public static List<Square> FindCandidate(int candidate, List<Square> squaresToSearch)
        {
            List<Square> whereFound = new List<Square>();
            foreach (Square s in squaresToSearch)
            {
                if (s.HasCandidate(candidate))
                {
                    whereFound.Add(s);
                }
            }
            return whereFound;
        }
        /// <summary>
        /// A dead end occurs if any square has no candidate values
        /// </summary>
        public bool isDeadEnd
        {
            get
            {
                for (int x = 0; x != boardLength; x++)
                {
                    for (int y = 0; y != boardLength; y++)
                    {
                        if (theBoard[x, y].hasNoCandidates)
                        {
                            return true;

                        }
                    }
                }
                return false;
            }
        }

        /// <summary>
        /// Check to see if the Sudoku has been solved
        /// </summary>
        public bool hasBeenSolved
        {
            get
            {
                List<Square> squares = null;
                // Check the rows
                for (int y = 0; y != boardLength; y++)
                {
                    squares = GetRow(y);
                    if (!SetIsComplete(squares))
                    {
                        return false;
                    }
                }
                // Check the columns
                for (int x = 0; x != boardLength; x++)
                {
                    squares = GetColumn(x);
                    if (!SetIsComplete(squares))
                    {
                        return false;
                    }
                }
                // Check the sectors
                for (int x = 0; x != boardLength; x += sectorLength)
                {
                    for (int y = 0; y != boardLength; y += sectorLength)
                    {
                        squares = GetSector(x, y);
                        if (!SetIsComplete(squares))
                        {
                            return false;
                        }
                    }
                }
                return true;
            }
        }
    }
}