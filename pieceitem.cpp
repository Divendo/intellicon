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

#include "pieceitem.h"
#include <QPainter>
#include <QTimerEvent>
#include <QtGlobal>

// Public:
    PieceItem::PieceItem(const int& col, const int& targetRow, const bool &isRed)
    : column(col), targetRow(targetRow), isRed(isRed), sprite(isRed ? ":/sprites/red-piece.png" : ":/sprites/yellow-piece.png"),
    animationStep(0), fallTimerID(0),
    winnerPiece(false), winnerAlpha(0), winnerAlphaDelta(4), winnerTimerID(0)
    {
        setPos(col * PieceItem::squareSize, 0);
        fallTimerID = startTimer(1000 / 30);    // 30 fps
    }

    QRectF PieceItem::boundingRect() const
    { return QRectF(0, 0, PieceItem::squareSize, PieceItem::squareSize); }

    void PieceItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
    {
        painter->drawPixmap(0, 0, PieceItem::squareSize, PieceItem::squareSize, sprite);
        if(winnerPiece)
            painter->fillRect(0, 0, PieceItem::squareSize, PieceItem::squareSize, QBrush(QColor(255, 255, 255, 15 + static_cast<int>(2.55 * winnerAlpha))));
    }

    void PieceItem::setWinnerPiece(const bool& isWinner)
    {
        if(winnerPiece = isWinner)
            winnerTimerID = startTimer(1000 / 30);
        else if(winnerTimerID != 0)
        {
            killTimer(winnerTimerID);
            winnerTimerID = 0;
            winnerAlpha = 0;
            winnerAlphaDelta = 4;
            update();
        }
    }

    int PieceItem::col() const
    { return column; }
    int PieceItem::row() const
    { return targetRow; }

// Protected:
    void PieceItem::timerEvent(QTimerEvent* event)
    {
        if(event->timerId() == fallTimerID)
        {
            const int targetY = (5 - targetRow) * PieceItem::squareSize;
            ++animationStep;

            // distance = 0.5 * acceleration * time^2
            // acceleration = 5.76, for falling 6 squares should take 1/3 second, so:
            //  s = 0.5 * a * t^2           =>
            //  6 * 48 = 0.5 * a * 10^2     => (30 steps per second)
            //  6 * 48 / (0.5 * 10^2) = 5.76
            // The 0.5 * acceleration part can now be replaced by 0.5 * 5.76 = 2.88
            int newY = qRound(2.88 * animationStep * animationStep);
            if(newY >= targetY)
            {
                newY = targetY;
                killTimer(event->timerId());
                fallTimerID = 0;
                fallingAnimationFinished();
            }
            setY(newY);
            update();
        }
        else if(event->timerId() == winnerTimerID)
        {
            winnerAlpha += winnerAlphaDelta;
            if(winnerAlpha >= 60)
            {
                winnerAlpha = 60;
                winnerAlphaDelta = -winnerAlphaDelta;
            }
            else if(winnerAlpha <= 0)
            {
                winnerAlpha = 0;
                winnerAlphaDelta = -winnerAlphaDelta;
            }
            update();
        }
    }

// Private:
    // Static:
        const int PieceItem::squareSize = 48;
