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

#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include "gameboard.h"

class Player : public QObject
{
    Q_OBJECT

    public:
        Player(const QString& name = "", const bool& playerIsRed = true);

        QString name;

        bool isRed() const;

        virtual bool isHuman() const;
        
    signals:
        void doMove(const int& col);
        void enableMouseInput();
        void disableMouseInput();
        void statusUpdate(const QString& text);
        
    public slots:
        virtual void move(const Board& b) = 0;
        virtual void mouseClick(const GameBoard::MouseClickEvent& event);
        virtual void abortMoveRequest();
        virtual void moveUndone();

    private:
        bool playerIsRed;   // Whether the player is red or not
};

#endif // PLAYER_H
