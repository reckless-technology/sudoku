namespace Sudoku
{
    public struct Move
    {
        public int x;
        public int y;
        public int value;
    }
    /// <summary>
    /// Interface for a generic Sudoku solving algorithm.
    /// </summary>
    public interface ISudokuSolver
    {
        /// <summary>
        /// Index operator to allow direct 2D access to squares on the Sudoku board.
        /// get returns 0 if the square has no candidate values or is ambiguous.
        /// set places values in the board.
        /// set with a value of 0 resets a square to its initial state.
        /// </summary>
        int this[int x, int y]
        {
            get; 
            set;
        }

        /// <summary>
        /// The Sudoku puzzle is complete and correct.
        /// </summary>
        bool hasBeenSolved
        {
            get;
        }

        /// <summary>
        /// Get the next move and update the Solver state accordingly.
        /// </summary>
        /// <returns>true if a new move was found</returns>
        bool GetNextMove(ref Move move);

        /// <summary>
        /// Completely solve the puzzle.
        /// </summary>
        /// <returns>true on success</returns>
        bool Solve();
    }
}
