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

#include "bitboard.h"

// Public:
    BitBoard::BitBoard(const quint64& boardInt)
    : bitmap(0), bitmapRed(0), bitmapYellow(0)
    { setBitBoard(boardInt); }

    void BitBoard::setBitBoard(const quint64& boardInt)
    {
        // Copy the board to our bitmap
        bitmap = boardInt;

        // Reset the red and yellow board
        bitmapRed = 0;
        bitmapYellow = 0;

        // Read the red and yellow board from the bitmap
        const quint64 col1 = (Q_UINT64_C(1) << 6) - 1;  // A set of bits where the bits 0...5 are true (i.e. one entire column of true bits, exclusive the top-bit)
        for(int col = 0; col < 7; ++col)
        {
            // Determine which color the highest piece in the column has
            const bool colIsRed = bitmap & (Q_UINT64_C(1) << (6 + 7 * col));
            quint64& color = colIsRed ? bitmapRed : bitmapYellow;
            quint64& opponent = colIsRed ? bitmapYellow : bitmapRed;

            // Determine in which row the highest piece currently is placed
            highestPieces[col] = 5;
            for(; highestPieces[col] >= 0; --highestPieces[col])
            {
                if(bitmap & (Q_UINT64_C(1) << (highestPieces[col] + 7 * col))) break;
            }

            // Copy the bits of the current column to the right ColorBoard
            color |= bitmap & (col1 << 7 * col);
            // Copy the inverse bits of the current column to the opponent ColorBoard
            // But only the bits below and including the highest piece should be inverted
            opponent |= (bitmap & (col1 << 7 * col)) ^ (((Q_UINT64_C(1) << (highestPieces[col] + 1)) - 1) << 7 * col);
        }

        // Count the number of bits in the red ColorBoard
        red2move = bitcount(bitmapRed) == bitcount(bitmapYellow);
    }
    void BitBoard::setBitBoard(const Board& board)
    {
        // Convert the board to an int and read the board from the generated int
        setBitBoard(BitBoard::board2int(board));
    }

    BitBoard::operator quint64() const
    { return bitmap; }
    const quint64 &BitBoard::toInt() const
    { return bitmap; }
    const quint64& BitBoard::redToInt() const
    { return bitmapRed; }
    const quint64& BitBoard::yellowToInt() const
    { return bitmapYellow; }

    Board BitBoard::toBoard() const
    {
        Board out(7, std::vector<Piece>(6, Empty));
        for(int col = 0; col < 7; ++col)
        {
            for(int row = 0; row < 6; ++row)
            {
                if(bitmapRed & (Q_UINT64_C(1) << (row + 7 * col)))
                    out[col][row] = Red;
                else if(bitmapYellow & (Q_UINT64_C(1) << (row + 7 * col)))
                    out[col][row] = Yellow;
            }
        }
        return out;
    }

    bool BitBoard::redToMove() const
    { return red2move; }
    bool BitBoard::canMove(const int& col) const
    { return BitBoard::canMove(bitmap, col); }
    bool BitBoard::redHasWon() const
    { return BitBoard::isWinner(bitmapRed); }
    bool BitBoard::yellowHasWon() const
    { return BitBoard::isWinner(bitmapYellow); }
    bool BitBoard::isFull() const
    { return BitBoard::isFull(bitmap); }

    int BitBoard::playableRow(const int& col) const
    { return highestPieces[col] == 5 ? -1 : highestPieces[col] + 1; }

    quint64 BitBoard::move(const int& col) const
    { return BitBoard::move(bitmap, col, red2move); }

    quint64 BitBoard::flip() const
    { return BitBoard::flip(bitmap); }

    // Returns the amount of pieces on the board
    int BitBoard::pieceCount() const
    { return BitBoard::bitcount(bitmapRed | bitmapYellow); }

    // Static:
        bool BitBoard::isWinner(const quint64& colorBoard)
        {
            // 283691315109952 is in binary: 1000000 1000000 1000000 1000000 1000000 1000000 1000000
            // In other words: all top-bits are true
            const quint64 topBits = Q_UINT64_C(283691315109952);
            if(topBits & colorBoard) return false;

            const quint64 diagRight = colorBoard & (colorBoard >> 6);   // Used for checking on groups that go from north-west to south-east
            const quint64 diagLeft  = colorBoard & (colorBoard >> 8);   // Used for checking on groups that go from north-east to south-west
            const quint64 hor       = colorBoard & (colorBoard >> 7);   // Used for checking on horizontal groups
            const quint64 vert      = colorBoard & (colorBoard >> 1);   // Used for checking on vertical groups

            return  (diagRight  & (diagRight >> 6 * 2)) |
                    (diagLeft   & (diagLeft  >> 8 * 2)) |
                    (hor        & (hor       >> 7 * 2)) |
                    (vert       & (vert      >> 2));
        }

        bool BitBoard::canMove(const quint64& bitmap, const int& col)
        {
            // If the bit at (5 + 7 * col) is set, then this column is full
            return !(bitmap & (Q_UINT64_C(1) << (5 + 7 * col)));
        }

        int BitBoard::playableRow(const quint64& bitmap, const int& col)
        {
            // 63 is in binary: 0111111
            // In other words: a completely filled column
            const quint64 colBits = ((Q_UINT64_C(63) << 7 * col) & bitmap) >> 7 * col;

            if(colBits & (1 << 5)) return -1;
            if(colBits & (1 << 4)) return 5;
            if(colBits & (1 << 3)) return 4;
            if(colBits & (1 << 2)) return 3;
            if(colBits & (1 << 1)) return 2;
            if(colBits & (1 << 0)) return 1;
            return 0;
        }

        quint64 BitBoard::move(const quint64& bitmap, const int& col, const bool& redToMove)
        {
            // Check if the move can be made
            if(!BitBoard::canMove(bitmap, col)) return bitmap;

            // Determine which color the highest piece in the column has
            const bool colIsRed = bitmap & (Q_UINT64_C(1) << (6 + 7 * col));

            // Determine in which row the highest piece currently is placed
            int highestPiece = 5;
            for(; highestPiece >= 0; --highestPiece)
            {
                if(bitmap & (Q_UINT64_C(1) << (highestPiece + 7 * col))) break;
            }

            // This will be our result, just copy the current board to it
            quint64 result = bitmap;

            // If the color of the target column isn't the same as the player that is to move
            // then we need to switch the color of that column
            if(colIsRed != redToMove)
            {
                // (Q_UINT64_C(1) << (6 + 7 * col)) creates a mask with a true bit on the top-bit
                // (((Q_UINT64_C(1) << (highestPiece + 1)) - 1) << 7 * col) creates a mask where all bits representing the squares that are filled in the column that's being played are true
                // Combining these two masks gives a mask that will change (using XOR) the color of the column that's being played
                result ^= (Q_UINT64_C(1) << (6 + 7 * col)) | (((Q_UINT64_C(1) << (highestPiece + 1)) - 1) << 7 * col);
            }

            // Add the played piece to the board
            result |= Q_UINT64_C(1) << (highestPiece + 1 + col * 7);

            // Return the result
            return result;
        }
        quint64 BitBoard::move(const quint64& bitmap, const int& col, const int& row, const bool& redToMove)
        {
            // Determine which color the highest piece in the column has
            const bool colIsRed = bitmap & (Q_UINT64_C(1) << (6 + 7 * col));

            // This will be our result, just copy the current board to it
            quint64 result = bitmap;

            // If the color of the target column isn't the same as the player that is to move
            // then we need to switch the color of that column
            if(colIsRed != redToMove)
            {
                // (Q_UINT64_C(1) << (6 + 7 * col)) creates a mask with a true bit on the top-bit
                // (((Q_UINT64_C(1) << row) - 1) << 7 * col) creates a mask where all bits representing the squares that are filled in the column that's being played are true
                // Combining these two masks gives a mask that will change (using XOR) the color of the column that's being played
                result ^= (Q_UINT64_C(1) << (6 + 7 * col)) | (((Q_UINT64_C(1) << row) - 1) << 7 * col);
            }

            // Add the played piece to the board
            result |= Q_UINT64_C(1) << (row + col * 7);

            // Return the result
            return result;
        }

        quint64 BitBoard::flip(const quint64& bitmap)
        {
            // Start with an empty board
            quint64 out = 0;

            // A set of bits where the bits 0...6 are true (i.e. one entire column of true bits, inclusive the top-bit)
            const quint64 col1 = (Q_UINT64_C(1) << 7) - 1;

            // Per column we copy the opposite column
            for(int col = 0; col < 7; ++col)
            {
                if(col < 3)
                    out |= (bitmap & (col1 << (7 * col))) << ((3 - col) * 2 * 7);
                else if(col == 3)
                    out |= bitmap & (col1 << (7 * col));
                else
                    out |= (bitmap & (col1 << (7 * col))) >> ((col - 3) * 2 * 7);
            }

            // Return the result
            return out;
        }
        bool BitBoard::isFull(const quint64& bitmap)
        {
            // 141845657554976 is in binary: 0100000 0100000 0100000 0100000 0100000 0100000 0100000
            // In other words: all bits in the sixth (top) row are true
            const quint64 topRow = Q_UINT64_C(141845657554976);
            return (bitmap & topRow) == topRow;
        }

        int BitBoard::pieceCount(const quint64& bitmapRed, const quint64& bitmapYellow)
        { return BitBoard::bitcount(bitmapRed | bitmapYellow); }

        quint64 BitBoard::board2int(const Board& board)
        {
            // Start with an empty board
            quint64 out = 0;
            for(int col = 0; col < 7; ++col)
            {
                Piece colColor = Empty;
                for(int row = 5; row >= 0; --row)
                {
                    if(board[col][row] == Empty) continue;

                    if(colColor == Empty)
                    {
                        // If the highest piece in this column is red, we set the top-bit to true
                        if((colColor = board[col][row]) == Red)
                            out |= Q_UINT64_C(1) << (6 + 7 * col);
                    }

                    // If the current piece is of the same color as the highest piece in this column
                    // then we set the corresponding bit to true
                    if(board[col][row] == colColor)
                        out |= Q_UINT64_C(1) << (row + 7 * col);
                }
            }

            // Return the result
            return out;
        }

        quint64 BitBoard::board2int(const std::string& str)
        {
            // Start with an empty board
            quint64 out = 0;
            for(int col = 0; col < 7; ++col)
            {
                char colColor = 0;
                for(int row = 5; row >= 0; --row)
                {
                    const char& chr = str[col * 6 + row];
                    if(chr == ' ') continue;

                    if(colColor == Empty)
                    {
                        // If the highest piece in this column is red, we set the top-bit to true
                        if((colColor = str[col * 6 + row]) == 'R')
                            out |= Q_UINT64_C(1) << (6 + 7 * col);
                    }

                    // If the current piece is of the same color as the highest piece in this column
                    // then we set the corresponding bit to true
                    if(str[col * 6 + row] == colColor)
                        out |= Q_UINT64_C(1) << (row + 7 * col);
                }
            }

            // Return the result
            return out;
        }

// Private:
    int BitBoard::bitcount(quint64 x)
    {
        // Some constants needed in this function
        const quint64 m1 = Q_UINT64_C(0x5555555555555555);  // Binary: 0101....
        const quint64 m2 = Q_UINT64_C(0x3333333333333333);  // Binary: 00110011...
        const quint64 m4 = Q_UINT64_C(0x0f0f0f0f0f0f0f0f);  // Binary: 0000111100001111...

        x -= (x >> 1) & m1;             // Put count of each 2 bits into those 2 bits
        x = (x & m2) + ((x >> 2) & m2); // Put count of each 4 bits into those 4 bits
        x = (x + (x >> 4)) & m4;        // Put count of each 8 bits into those 8 bits
        x += x >>  8;                   // Put count of each 16 bits into their lowest 8 bits
        x += x >> 16;                   // Put count of each 32 bits into their lowest 8 bits
        x += x >> 32;                   // Put count of each 64 bits into their lowest 8 bits
        return x & 0x7f;
    }
