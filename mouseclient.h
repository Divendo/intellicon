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

#ifndef COLUMNHIGHLIGHTER_H
#define COLUMNHIGHLIGHTER_H

#include <QGraphicsItem>
#include <QObject>

class MouseClient : public QObject, public QGraphicsItem
{
    Q_OBJECT

    public:
        enum State
        {
            None,
            Hover,
            Active,
            Disabled
        };

        MouseClient(const int& squareSize);

        QRectF boundingRect() const;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

        void setState(const State& newState);
        void disable();
        void enable();

    signals:
        void clicked(const int& x, const int& y);

    protected:
        void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent* event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

        void mousePressEvent(QGraphicsSceneMouseEvent* event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

    private:
        int squareSize;
        int highlightColumn;
        State state;
};

#endif // COLUMNHIGHLIGHTER_H
