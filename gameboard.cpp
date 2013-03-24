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

#include "gameboard.h"
#include <QPainter>
#include <QtCore/qmath.h>
#include <algorithm>

// Public:
    GameBoard::GameBoard(const int& width, const int& height)
    : boardForeground(":/sprites/board-foreground.png"), mouseClient(width / 7),
    redIsHuman(false), yellowIsHuman(false)
    {
        setSceneRect(0, 0, width, height);
        addItem(&mouseClient);
        connect(&mouseClient, SIGNAL(clicked(const int&, const int&)), this, SLOT(mouseClicked(const int&, const int&)));
        disableMouse();
        resetBoard();
    }

    void GameBoard::setRedHuman(const bool& isHuman)
    { redIsHuman = isHuman; }
    void GameBoard::setYellowHuman(const bool& isHuman)
    { yellowIsHuman = isHuman; }

// Public slots:
    void GameBoard::resetBoard()
    {
        // Clear the entire board
        board.clear();
        board.resize(7, std::vector<Piece>(6, Empty));

        // Remove the PieceItems from the memory
        for(std::list<PieceItem*>::iterator pos = pieceItems.begin(); pos != pieceItems.end(); ++pos)
        {
            removeItem(*pos);
            delete *pos;
        }
        pieceItems.clear();

        // Clear the moves history
        moves.clear();

        // The red player starts
        turn = Red;
    }

// Used for debugging, can be used in GameBoard::startGame() to set the board to a certain position
#define FAKE_MOVE(col, row, color) \
    board[col][row] = color; \
    pieceItems.push_back(new PieceItem(col, row, color == Red)); \
    addItem(pieceItems.back()); \
    moves.push_back(col); \
    enableUndoButton(true);

    void GameBoard::startGame()
    {
        if(!board.empty())  resetBoard();

        gameEnded = false;

        if(turn == Red)     redMoveRequest(board);
        else                yellowMoveRequest(board);
    }

    void GameBoard::redPlayCol(const int& col)
    {
        if(turn != Red) return;
        playCol(col);
    }

    void GameBoard::yellowPlayCol(const int& col)
    {
        if(turn != Yellow) return;
        playCol(col);
    }

    void GameBoard::enableMouse()
    { mouseClient.enable(); }

    void GameBoard::disableMouse()
    { mouseClient.disable(); }

    void GameBoard::undoMove()
    {
        const int col = moves.back();
        int row = 5;
        for(; row >= 0; --row)
            if(board[col][row] != Empty) break;

        if(gameEnded)
        {
            clearWinner();
            for(std::list<PieceItem*>::iterator pos = pieceItems.begin(); pos != pieceItems.end(); ++pos)
                (*pos)->setWinnerPiece(false);
            gameEnded = false;
        }

        board[col][row] = Empty;
        removeItem(pieceItems.back());
        delete pieceItems.back();

        moves.pop_back();
        pieceItems.pop_back();

        if(moves.size() == 0) enableUndoButton(false);

        if(turn == Red)
        {
            turn = Yellow;
            redAbortMoveRequest();
            if(moves.size() == 0 || yellowIsHuman)
            {
                yellowMoveRequest(board);
                moveUndone();
            }
            else
                undoMove();
        }
        else
        {
            turn = Red;
            yellowAbortMoveRequest();
            if(moves.size() == 0 || redIsHuman)
            {
                redMoveRequest(board);
                moveUndone();
            }
            else
                undoMove();
        }
    }

// Protected
    void GameBoard::drawBackground(QPainter* painter, const QRectF&)
    {
        // Save the painter state
        painter->save();

        // Make the entire background black
        painter->fillRect(0, 0, width(), height(), QColor::fromRgb(0, 0, 0));

        // Set a white brush and activate antialiasing
        painter->setBrush(QBrush(QColor::fromRgb(255, 255, 255)));
        painter->setRenderHint(QPainter::Antialiasing, true);

        // Draw the white circles
        for(int x = 0; x < 7; ++x)
        {
            for(int y = 0; y < 6; ++y)
                painter->drawEllipse(x * 48 + 4, y * 48 + 4, 43, 43);
        }

        // Restore the painter state
        painter->restore();
    }

    void GameBoard::drawForeground(QPainter* painter, const QRectF&)
    {
        // Draw the foreground image over each hole
        for(int x = 0; x < 7; ++x)
        {
            for(int y = 0; y < 6; ++y)
                painter->drawPixmap(x * 48, y * 48, 48, 48, boardForeground);
        }
    }

// Private:
    void GameBoard::checkForWinner(const int& col, const int& row)
    {
        const Piece currColor = board[col][row];

        // The amount of pieces of the same color in each direction
        // 0 = north, 1 = north-east, 2 = east, 3 = south-east, etc
        int pieces[] = {0, 0, 0, 0, 0, 0, 0, 0};

        // Whether each direction should be checked for more pieces of the same color
        // 0 = north, 1 = north-east, 2 = east, 3 = south-east, etc
        int directionCheck[] = {true, true, true, true, true, true, true, true};


        for(int i = 1; i <= 3; ++i)
        {
            // Check left from the last played piece
            if(col - i >= 0)
            {
                // Check below the last played piece
                if(row - i < 0 || !directionCheck[5] || board[col - i][row - i] != currColor)
                    directionCheck[5] = false;
                else
                    ++pieces[5];

                // Check next to the last played piece
                if(!directionCheck[6] || board[col - i][row] != currColor)
                    directionCheck[6] = false;
                else
                    ++pieces[6];

                // Check above the last played piece
                if(row + i >= 6 || !directionCheck[7] || board[col - i][row + i] != currColor)
                    directionCheck[7] = false;
                else
                    ++pieces[7];
            }
            else
            {
                directionCheck[7] = false;
                directionCheck[6] = false;
                directionCheck[5] = false;
            }

            // Check right from the last played piece
            if(col + i < 7)
            {
                if(row - i < 0 || !directionCheck[3] || board[col + i][row - i] != currColor)
                    directionCheck[3] = false;
                else
                    ++pieces[3];

                if(!directionCheck[2] || board[col + i][row] != currColor)
                    directionCheck[2] = false;
                else
                    ++pieces[2];

                if(row + i >= 6 || !directionCheck[1] || board[col + i][row + i] != currColor)
                    directionCheck[1] = false;
                else
                    ++pieces[1];
            }
            else
            {
                directionCheck[1] = false;
                directionCheck[2] = false;
                directionCheck[3] = false;
            }

            // Check vertically below the last played piece
            if(row - i < 0 || !directionCheck[4] || board[col][row - i] != currColor)
                directionCheck[4] = false;
            else
                ++pieces[4];

            // Check vertically above the last played piece
            if(row + i >= 6 || !directionCheck[0] || board[col][row + i] != currColor)
                directionCheck[0] = false;
            else
                ++pieces[0];
        }

        /**************
        * Directions: *
        * 7  0  1     *
        * 6     2     *
        * 5  4  3     *
        **************/

        // If in any of the directions 3 or more adjacent pieces of the same color are found, the player has connected 4 pieces
        // since we've to include the last played piece
        if(pieces[0] + pieces[4] >= 3 || pieces[1] + pieces[5] >= 3 || pieces[2] + pieces[6] >= 3 || pieces[3] + pieces[7] >= 3)
        {
            winner(currColor);
            disableMouse();
            gameEnded = true;

            // Find out which pieces are part of the winning line
            std::vector< std::pair<int, int> > winCoords;
            if(pieces[0] + pieces[4] >= 3)
            {
                for(int i = -pieces[4]; i <= pieces[0] && winCoords.size() < 4; ++i)
                    winCoords.push_back(std::make_pair(col, row + i));
            }
            else if(pieces[1] + pieces[5] >= 3)
            {
                for(int i = -pieces[5]; i <= pieces[1] && winCoords.size() < 4; ++i)
                    winCoords.push_back(std::make_pair(col + i, row + i));
            }
            else if(pieces[2] + pieces[6] >= 3)
            {
                for(int i = -pieces[6]; i <= pieces[2] && winCoords.size() < 4; ++i)
                    winCoords.push_back(std::make_pair(col + i, row));
            }
            else if(pieces[3] + pieces[7] >= 3)
            {
                for(int i = -pieces[7]; i <= pieces[3] && winCoords.size() < 4; ++i)
                    winCoords.push_back(std::make_pair(col + i, row - i));
            }

            // Mark the winning pieces
            int piecesMarked = 0;
            PieceItem* items[] = {0, 0, 0, 0};
            for(std::list<PieceItem*>::iterator pos = pieceItems.begin(); piecesMarked < 4 && pos != pieceItems.end(); ++pos)
            {
                if(std::find(winCoords.begin(), winCoords.end(), std::make_pair((*pos)->col(), (*pos)->row())) != winCoords.end())
                    items[piecesMarked++] = *pos;
            }
            for(int i = 0; i < 4; ++i)
                items[i]->setWinnerPiece(true);
        }
        else
        {
            // Check if the board is entirely filled
            bool boardFilled = true;
            for(int i = 0; boardFilled && i < 7; ++i)
                boardFilled = board[i][5] != Empty;

            // Since no player connected 4 pieces we call it a draw
            if(boardFilled)
            {
                winner(Empty);
                disableMouse();
                gameEnded = true;
            }
        }
    }

    bool GameBoard::playCol(const int& col)
    {
        // When this function returns, a move from the current player is always requested
        // Even if the function returns false
        // Only if the game has ended no move is requested

        // Game ended? Return false
        if(gameEnded) return false;

        // Invalid column? Return false
        if(col < 0 || col >= 7)
        {
            if(turn == Red) redMoveRequest(board);
            else            yellowMoveRequest(board);
            return false;
        }

        // Check in which row the piece will end
        int row = 0;
        for(; row < 6; ++row)
        {
            if(board[col][row] == Empty) break;
        }

        // If the chosen column is full, return false
        if(row == 6)
        {
            if(turn == Red) redMoveRequest(board);
            else            yellowMoveRequest(board);
            return false;
        }

        // Play the piece and return true
        board[col][row] = turn;
        pieceItems.push_back(new PieceItem(col, row, turn == Red));
        addItem(pieceItems.back());
        moves.push_back(col);
        enableUndoButton(true);
        turn = turn == Red ? Yellow : Red;
        checkForWinner(col, row);
        if(!gameEnded)
        {
            if(turn == Red) redMoveRequest(board);
            else            yellowMoveRequest(board);
        }
        return true;
    }

// Private slots:
    void GameBoard::mouseClicked(const int& x, const int& y)
    {
        const MouseClickEvent e(x, y, x / 48, 5 - y / 48);
        if(turn == Red) redClickEvent(e);
        else            yellowClickEvent(e);
    }
