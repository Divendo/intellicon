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

#ifndef PIECEITEM_H
#define PIECEITEM_H

#include <QGraphicsItem>
#include <QObject>

class PieceItem : public QObject, public QGraphicsItem
{
    Q_OBJECT

    public:
        PieceItem(const int& col, const int& targetRow, const bool& isRed);

        QRectF boundingRect() const;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

        void setWinnerPiece(const bool& isWinner);

        int col() const;
        int row() const;

    signals:
        void fallingAnimationFinished();

    protected:
        void timerEvent(QTimerEvent* event);

    private:
        static const int squareSize;

        int column;             // The column of the piece
        int targetRow;          // The row that the piece should end in
        bool isRed;             // Whether the piece is red or yellow
        QPixmap sprite;
        int animationStep;      // Used for the fall animation, to keep track of the amount of steps
        int fallTimerID;        // The ID of the timer used for the falling animation

        bool winnerPiece;       // Whether this piece is part of the winning line
        int winnerAlpha;        // The alpha of the white square that's drawn over this piece to indicate that it's part of the winning line
        int winnerAlphaDelta;   // The amount of alpha that winnerAlpha should be increased on every time step
        int winnerTimerID;      // The ID of the timer used for the winner animation
};

#endif // PIECEITEM_H
