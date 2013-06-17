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

#include "perfectplayer.h"

// Public:
    PerfectPlayer::PerfectPlayer(const QString& name, const bool& playerIsRed)
    : Player(name, playerIsRed), thread(playerIsRed)
    {
        qRegisterMetaType<StatusPhase>("StatusPhase");

        connect(&threadManager, SIGNAL(started()), &thread, SLOT(searchMove()));
        connect(&thread, SIGNAL(doMove(const int&)), this, SLOT(moveFound(const int&)));
        connect(&thread, SIGNAL(statusUpdate(const StatusPhase&,const int&)), this, SLOT(statusUpdateReceiver(const StatusPhase&, const int&)));

        thread.moveToThread(&threadManager);
    }

// Public slots:
    void PerfectPlayer::move(const Board& b)
    {
        thread.setBoard(b);
        threadManager.start();
    }

    void PerfectPlayer::abortMoveRequest()
    {
        thread.stop();
        threadManager.quit();
        threadManager.wait();
    }

// Private slots:
    void PerfectPlayer::moveFound(const int& col)
    {
        threadManager.quit();
        threadManager.wait();
        doMove(col);
    }

    void PerfectPlayer::statusUpdateReceiver(const StatusPhase& phase, const int& n)
    {
        QString msg;
        switch(phase)
        {
            case FindingPlayableCols:
                msg = tr("Speelbare kolommen zoeken...");
            break;
            case TryingWinningMove:
                msg = tr("Winnende zet proberen...");
            break;
            case BlockingLosingMove:
                msg = tr("Verliezende zet voorkomen...");
            break;
            case SearchingSolutions:
                //: At %1 the progress will be inserted as a percentage
                msg = tr("Oplossingen zoeken (%1%)...").arg(100 * n / 7);
            break;
            case TreeSearching:
                if(n == - 1)
                    msg = tr("Zetten vooruit denken...");
                else
                {
                    //: At %1 the progress will be inserted as a percentage
                    msg = tr("Zetten vooruit denken (%1%)...").arg(n);
                }
            break;
            case ChoosingAMove:
                msg = tr("Een zet kiezen aan de hand van de gevonden oplossingen...");
            break;
        }
        statusUpdate(msg);
    }
