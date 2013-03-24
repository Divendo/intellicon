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

#include "dumbplayer.h"
#include "bitboard.h"

// Public:
    DumbPlayer::DumbPlayer(const QString& name, const bool& playerIsRed)
    : Player(name, playerIsRed)
    { }

// Public slots:
    void DumbPlayer::move(const Board& board)
    {
        // Create a BitBoard from the board
        const BitBoard bitBoard(BitBoard::board2int(board));

        // If we can complete a group, we make that move
        const quint64& colorBoard = isRed() ? bitBoard.redToInt() : bitBoard.yellowToInt();
        std::vector<int> moveableCols;
        for(int col = 0; col < 7; ++col)
        {
            if(!bitBoard.canMove(col)) continue;

            moveableCols.push_back(col);

            const int row = bitBoard.playableRow(col);
            if(BitBoard::isWinner( colorBoard | (Q_UINT64_C(1) << (row + col * 7)) ))
            {
                doMove(col);
                return;
            }
        }

        // Determine the threats of the opponent
        // If he can complete one we block that threat
        const quint64& otherColorBoard = isRed() ? bitBoard.yellowToInt() : bitBoard.redToInt();
        std::vector<int> forcedCols;
        for(unsigned int i = 0; i < moveableCols.size(); ++i)
        {
            const int row = bitBoard.playableRow(moveableCols[i]);
            if(BitBoard::isWinner( otherColorBoard | (Q_UINT64_C(1) << (row + moveableCols[i] * 7)) ))
                forcedCols.push_back(moveableCols[i]);
        }

        // If there are forced moves we randomly pick a move from them
        if(!forcedCols.empty())
            doMove(forcedCols[qrand() % forcedCols.size()]);

        // Pick a random move
        doMove(moveableCols[qrand() % moveableCols.size()]);
    }
