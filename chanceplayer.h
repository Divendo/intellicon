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

#ifndef EASYPLAYER_H
#define EASYPLAYER_H

#include "perfectplayer.h"
#include "dumbplayer.h"

class ChancePlayer : public PerfectPlayer
{
    Q_OBJECT

    public:
        ChancePlayer(const QString& name = "", const bool& playerIsRed = true, const int& chanceDenominator = 1, const int& chanceDivisor = 4);

    public slots:
        void move(const Board& b);
        void abortMoveRequest();

    private:
        DumbPlayer dumbPlayer;
        int chanceDenominator;
        int chanceDivisor;
};

#endif // EASYPLAYER_H
