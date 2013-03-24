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

#include "chanceplayer.h"
#include <ctime>
#include <QtGlobal>

// Public:
    ChancePlayer::ChancePlayer(const QString& name, const bool& playerIsRed, const int& chanceDenominator, const int& chanceDivisor)
    : PerfectPlayer(name, playerIsRed), dumbPlayer(name, playerIsRed), chanceDenominator(chanceDenominator), chanceDivisor(chanceDivisor)
    {
        // Seed the randomizer
        qsrand(time(0));

        // Pass the results from dumbPlayer through
        connect(&dumbPlayer, SIGNAL(doMove(const int&)), this, SIGNAL(doMove(const int&)));
    }

// Public slots:
    void ChancePlayer::move(const Board& b)
    {
        // Chance of chanceDenominator/diceSides to think about a move
        if(qrand() % chanceDivisor < chanceDenominator)
            PerfectPlayer::move(b);
        else
            dumbPlayer.move(b);
    }

    void ChancePlayer::abortMoveRequest()
    {
        PerfectPlayer::abortMoveRequest();
        dumbPlayer.abortMoveRequest();
    }
