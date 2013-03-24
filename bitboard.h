/************************************************************************
* This file is part of IntelliCon.                                      *
*                                                                       *
* IntelliCon is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation, either version 3 of the License, or     *
* (at your option) any later version.                                   *
*                                                                       *
* IntelliCon is distributed in the hope that it will be useful,         *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
* GNU General Public License for more details.                          *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* along with IntelliCon.  If not, see <http://www.gnu.org/licenses/>.   *
************************************************************************/

#ifndef BITBOARD_H
#define BITBOARD_H

#include "gameboard.h"
#include <QtGlobal>
#include <string>

/** BoardInt: an int describing a board:
  The bits describe the following squares on the board:
  .  .  .  .  .  .  .
  5 12 19 26 33 40 47   Top of the board (sixth row)
  4 11 18 25 32 39 46
  3 10 17 24 31 38 45
  2  9 16 23 30 37 44
  1  8 15 22 29 36 43
  0  7 14 21 28 35 42   Bottom of the board (first row)

  The dots on the place of bit 6, 13, 20, 27, 34, 41 and 48 have a special meaning (they're called top-bits).
  They tell us which color true bits represent in that column.
  If the top-bit is true all true bits in that column represent red pieces, otherwise the represent yellow bits.
  The highest filled square in a column is always a true bit.
  The false bits below this square represent the other color.
  The false bits above the highest true bit are empty squares.

  If a column is empty the value of the top-bit is meaningless.
  The top-bit should then always be false in order to make sure a position always has the same value.
**/

/** ColorBoard: an int describing a board for one color:
  The bits describe the following squares on the board:
  .  .  .  .  .  .  .
  5 12 19 26 33 40 47   Top of the board (sixth row)
  4 11 18 25 32 39 46
  3 10 17 24 31 38 45
  2  9 16 23 30 37 44
  1  8 15 22 29 36 43
  0  7 14 21 28 35 42   Bottom of the board (first row)

  A true bit represents a piece of us in that square.
  A false bit represents an empty square or a piece of the other color in that square.

  The dots on the place of bit 6, 13, 20, 27, 34, 41 and 48 (they're called top-bits) are always false.
  They're usefull for determining if the player has won for example.
**/

// Wrapper class for using an int as a connect-4 board
class BitBoard
{
    public:
        // Constructs the BitBoard from the given int
        BitBoard(const quint64& boardInt);

        // Loads the board from the given int
        void setBitBoard(const quint64& boardInt);
        // Loads the board from the given Board
        void setBitBoard(const Board& board);

        // Return the board as an int
        operator quint64() const;
        // Return the board as an int
        const quint64& toInt() const;
        // Return the board for red as an int
        const quint64& redToInt() const;
        // Return the board for yellow as an int
        const quint64& yellowToInt() const;

        // Returns the board as a Board
        Board toBoard() const;

        // Whether red is to move or not
        bool redToMove() const;
        // Returns whether the given move can be made
        bool canMove(const int& col) const;
        // Returns whether red has won this position
        bool redHasWon() const;
        // Returns whether yellow has won this position
        bool yellowHasWon() const;
        // Returns whether the board is completely filled
        bool isFull() const;

        // Returns the row of the direct playable square in the given column
        // Returns -1 if the square isn't playable
        int playableRow(const int& col) const;

        // Returns a new board where the given move is made (if possible)
        quint64 move(const int& col) const;
        // Returns the board flipped horizontally
        quint64 flip() const;

        // Returns the amount of pieces on the board
        int pieceCount() const;

        // Returns if the colorBoard contains a winning group
        // Returns false if any of the top-bits of colorBoard is true
        static bool isWinner(const quint64& colorBoard);

        // Returns whether a move can be made in the given column on the given board
        static bool canMove(const quint64& bitmap, const int& col);
        // Returns a new board where the given move is made (if possible)
        static quint64 move(const quint64& bitmap, const int& col, const bool& redToMove);
        // Returns the board flipped horizontally
        static quint64 flip(const quint64& bitmap);

        // Converts the given board to an int
        static quint64 board2int(const Board& board);
        // Converts the given board (as a string) to an int
        static quint64 board2int(const std::string& board);

    private:
        quint64 bitmap;
        quint64 bitmapRed;
        quint64 bitmapYellow;
        bool red2move;
        int highestPieces[7];

        // Counts the number of set bits
        int bitcount(quint64 x) const;
};

#endif // BITBOARD_H
