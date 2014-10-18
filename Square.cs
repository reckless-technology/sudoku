using System;
namespace Sudoku
{
    class Square
    {
        /// <summary>
        /// Location on the board
        /// </summary>
        public readonly int x, y;
        /// <summary>
        /// The presence of candidate values 
        /// is determined in a bitset contained
        /// within a 32 bit unsigned integer:
        /// 
        /// bit 0 = candidate 1
        /// bit 8 = candidate 9
        /// 
        /// This precludes the use of a Sudoku board size > 32 x 32,
        /// but is done to improve efficiency of candidate manipulation.
        /// </summary>
        private uint candidates;
        /// <summary>
        /// Constructor
        /// </summary>
        public Square(int howManyCandidates, int xLocation, int yLocation)
        {
            x = xLocation;
            y = yLocation;
            candidates = (1u << howManyCandidates) - 1;
        }
        /// <summary>
        /// Copy constructor
        /// </summary>
        public Square(Square other)
        {
            candidates = other.candidates;
            x = other.x;
            y = other.y;
        }

        public bool hasNoCandidates
        {
            get
            {
                return (candidates == 0);
            }
        }

        public bool hasUniqueValue
        {
            get
            {
                return ((candidates & (candidates - 1)) == 0 && candidates != 0);
            }
        }

        public int numberOfCandidates
        {
            get
            {
                return Cardinality(candidates);
            }
        }

        public int value
        {
            get
            {
                if (!hasUniqueValue)
                {
                    throw new ApplicationException("No value to get");
                }
                return this[0];
            }
        }
        /// <summary>
        /// Readonly access to candidate values, based on candidate index.
        /// Use this[n] to get the value of the n-th candidate (zero based index)
        /// </summary>
        public int this[int index]
        {
            get
            {
                int value = 1;
                uint temp = candidates;
                while (temp != 0)
                {
                    if ((temp & 1) != 0)
                    {
                        if (index == 0)
                        {
                            return value;
                        }
                        else
                        {
                            index--;
                        }
                    }
                    value++;
                    temp >>= 1;
                }
                throw new ApplicationException("Cannot get value");
            }
        }

        public void Assign(int value)
        {
            candidates = 1u << (value - 1);
        }

        public bool HasCandidate(int value)
        {
            return ((candidates & (1u << (value - 1))) != 0);
        }
        /// <summary>
        /// Eliminate a candidate by value
        /// </summary>
        /// <returns>true if the value was present and has been eliminated; otherwise false</returns>
        public bool Eliminate(int value)
        {
            if (!HasCandidate(value))
            {
                return false;
            }
            candidates &= ~(1u << (value - 1));
            return true;
        }

        public bool HasAllOf(uint bitset)
        {
            return ((bitset & candidates) == bitset);
        }

        public bool HasAnyOf(uint bitset)
        {
            return ((bitset & candidates) != 0);
        }

        public bool HasOnly(uint bitset)
        {
            return (bitset == candidates);
        }

        /// <summary>
        /// AND the candidate values with a bitset mask
        /// </summary>
        /// <param name="mask">mask to use</param>
        /// <returns>true if any candidates were removed in the AND</returns>
        public bool AndMask(uint mask)
        {
            uint temp = candidates;
            candidates &= mask;
            return (temp != candidates);
        }
        /// <summary>
        /// Compute the cardinality of an unsigned int
        /// </summary>
        /// <returns>number of bits set in bitset</returns>
        public static int Cardinality(uint bitset)
        {
            int howManyBits = 0;
            while (bitset != 0)
            {
                if ((bitset & 1) != 0)
                {
                    howManyBits++;
                }
                bitset >>= 1;
            }
            return howManyBits;
        }
    }
}
