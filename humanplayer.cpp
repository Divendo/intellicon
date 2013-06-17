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

#include "humanplayer.h"

// Public:
    HumanPlayer::HumanPlayer(const QString& name, const bool& playerIsRed)
    : Player(name, playerIsRed)
    { }

    bool HumanPlayer::isHuman() const
    { return true; }

// Public slots:
    void HumanPlayer::move(const Board& b)
    {
        board = b;
        enableMouseInput();
    }

    void HumanPlayer::mouseClick(const GameBoard::MouseClickEvent& event)
    {
        if(board[event.col][5] != Empty) return;

        board.clear();
        doMove(event.col);
    }