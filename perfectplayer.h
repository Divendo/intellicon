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

#ifndef PERFECTPLAYER_H
#define PERFECTPLAYER_H

#include "player.h"
#include "perfectplayerthread.h"
#include <QThread>

class PerfectPlayer : public Player
{
    Q_OBJECT

    public:
        PerfectPlayer(const QString& name = "", const bool& playerIsRed = true);

    public slots:
        void move(const Board& b);
        void abortMoveRequest();

    private:
        PerfectPlayerThread thread;
        QThread threadManager;

    private slots:
        void moveFound(const int& col);
        void statusUpdateReceiver(const StatusPhase& phase, const int& n);
};

#endif // PERFECTPLAYER_H
