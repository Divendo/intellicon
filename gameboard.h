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

#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <QGraphicsScene>
#include <QPixmap>
#include <list>
#include <vector>
#include "mouseclient.h"
#include "pieceitem.h"

enum Piece
{
    Empty   = 0,
    Red     = 1,
    Yellow  = 2
};

typedef std::vector< std::vector<Piece> > Board;

class GameBoard : public QGraphicsScene
{
    Q_OBJECT

    public:
        struct MouseClickEvent
        {
            MouseClickEvent(const int& x, const int& y, const int& col, const int& row)
            : x(x), y(y), col(col), row(row) {}

            // The x and y coordinate where the mouse clicked and which column and row that is
            int x, y, col, row;
        };

        GameBoard(const int& width, const int& height);

        void setRedHuman(const bool& isHuman);
        void setYellowHuman(const bool& isHuman);

    signals:
        void redMoveRequest(const Board& board);
        void yellowMoveRequest(const Board& board);

        void redClickEvent(const GameBoard::MouseClickEvent& event);
        void yellowClickEvent(const GameBoard::MouseClickEvent& event);

        void redAbortMoveRequest();
        void yellowAbortMoveRequest();

        void moveUndone();

        // Sent when a player won, or Empty if it is a draw
        void winner(const Piece& color);

        // Sent when the winner text should be cleared
        void clearWinner();

        // Sent when the undo button should be enabled or disabled
        void enableUndoButton(const bool& enable);

    public slots:
        // Resets the board
        void resetBoard();

        // Starts the game
        void startGame();

        // Let red play a piece in the given column
        void redPlayCol(const int& col);

        // Let yellow play a piece in the given column
        void yellowPlayCol(const int& col);

        // Enable mouse input
        void enableMouse();

        // Disable mouse input
        void disableMouse();

        // Undo the last move
        void undoMove();

    protected:
        void drawBackground(QPainter* painter, const QRectF& rect);
        void drawForeground(QPainter* painter, const QRectF& rect);

    private:
        QPixmap boardForeground;
        MouseClient mouseClient;
        Board board;                        // 2D array containing the current state of the board (in the form board[x][y] = Empty/Red/Yellow)
        Piece turn;                         // Whose turn it currently is
        std::list<PieceItem*> pieceItems;   // List of PieceItems that are currently on the board
        bool gameEnded;                     // Whether the game has ended or not
        std::list<int> moves;               // List of columns where moves were played
        bool redIsHuman;                    // Whether or not the red player is a human player
        bool yellowIsHuman;                 // Whether or not the yellow player is a human player

        // Checks if the game is won by someone
        // col and row are the column and row where the last piece was played
        void checkForWinner(const int& col, const int& row);

        // Put a piece in the given column (if possible), returns whether it was possible or not
        bool playCol(const int& col);

    private slots:
        void mouseClicked(const int& x, const int& y);
};

#endif // GAMEBOARD_H
