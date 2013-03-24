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

#include "mouseclient.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QtCore/qmath.h>

// Public:
    MouseClient::MouseClient(const int& squareSize)
    : squareSize(squareSize), highlightColumn(-1), state(None)
    {
        setAcceptHoverEvents(true);
    }

    QRectF MouseClient::boundingRect() const
    { return QRectF(0, 0, 7 * squareSize, 6 * squareSize); }

    void MouseClient::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
    {
        // If the mouse isn't hovering or active, nothing should be shown
        if(state == None || state == Disabled || highlightColumn == -1) return;

        // Determine what color we need to show (hover: #ddd, active: #aaa) and create a brush
        const int color = state == Hover ? 0xdd : 0xaa;
        QBrush brush(QColor::fromRgb(color, color, color, 102));    // Alpha = 0.4

        // Draw the rectangle
        painter->fillRect(squareSize * highlightColumn, 0, squareSize, squareSize * 6, brush);
    }

    void MouseClient::setState(const State& newState)
    {
        if(state == Disabled) return;

        if(state == newState) return;
        state = newState;
        update();
    }

    void MouseClient::disable()
    {
        state = Disabled;
        update();
    }

    void MouseClient::enable()
    { state = None; }

// Protected:
    void MouseClient::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
    {
        if(state == Disabled) return;

        const QPointF pos = event->scenePos();

        highlightColumn = qFloor(pos.x() / squareSize);
        setState(Hover);
    }
    void MouseClient::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
    {
        if(state == Disabled) return;

        const QPointF pos = event->scenePos();

        highlightColumn = qFloor(pos.x() / squareSize);
        update();
    }
    void MouseClient::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
    {
        if(state == Disabled) return;

        setState(None);
        highlightColumn = -1;
    }

    void MouseClient::mousePressEvent(QGraphicsSceneMouseEvent* event)
    {
        if(state == Disabled) return;

        const QPointF pos = event->scenePos();

        highlightColumn = qFloor(pos.x() / squareSize);
        setState(Active);
    }

    void MouseClient::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
    {
        if(state == Disabled) return;

        const QPointF pos = event->scenePos();

        if(highlightColumn != qFloor(pos.x() / squareSize))
            setState(None);
        else if(highlightColumn == qFloor(pos.x() / squareSize))
            setState(Active);
    }

    void MouseClient::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
    {
        if(state == Disabled) return;

        const QPointF pos = event->scenePos();

        // Only if the state was still Active we send a click event
        if(state == Active)
            clicked(pos.x(), pos.y());
        setState(Hover);
    }
