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

#include "gamewindow.h"
#include "ui_gamewindow.h"

// Public:
    GameWindow::GameWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::GameWindow), board(336, 288),
      redPlayer(0), yellowPlayer(0)
    {
        // Set up ui
        ui->setupUi(this);
        this->setStyleSheet("QMainWindow { background-color: #eee; }");

        // Activate the scene for the game
        ui->graphGame->setScene(&board);
        connect(&board, SIGNAL(winner(const Piece&)), this, SLOT(winner(const Piece&)));
        connect(&board, SIGNAL(clearWinner()), this, SLOT(clearWinner()));
        connect(&board, SIGNAL(enableUndoButton(const bool&)), this, SLOT(enableUndoButton(const bool&)));
        connect(&board, SIGNAL(redMoveRequest(const Board&)), ui->statusBar, SLOT(clearMessage()));
        connect(&board, SIGNAL(yellowMoveRequest(const Board&)), ui->statusBar, SLOT(clearMessage()));
        connect(&board, SIGNAL(winner(const Piece&)), ui->statusBar, SLOT(clearMessage()));

        // Hide this screen when going back to the menu
        connect(this, SIGNAL(backToMenu()), this, SLOT(hide()));
    }

    GameWindow::~GameWindow()
    {
        if(redPlayer != 0)      delete redPlayer;
        if(yellowPlayer != 0)   delete yellowPlayer;
        delete ui;
    }

// Public slots:
#include "linethreat.h"

    bool f(const ThreatSolution* a, const ThreatSolution* b)
    { return !a->interferesColumnWise(b); }

    void GameWindow::startGame(Player* red, Player* yellow)
    {
        enableUndoButton(false);
        connectRed(red);
        connectYellow(yellow);
        clearWinner();
        board.startGame();
        show();
    }

// Private:
    void GameWindow::connectRed(Player* player)
    {
        if(redPlayer != 0) delete redPlayer;
        redPlayer = player;
        ui->labelP1Name->setText(redPlayer->name);

        board.setRedHuman(player->isHuman());

        connect(&board, SIGNAL(redMoveRequest(const Board&)), player, SLOT(move(const Board&)));
        connect(&board, SIGNAL(redClickEvent(const GameBoard::MouseClickEvent&)), player, SLOT(mouseClick(const GameBoard::MouseClickEvent&)));
        connect(&board, SIGNAL(moveUndone()), player, SLOT(moveUndone()));
        connect(&board, SIGNAL(redAbortMoveRequest()), player, SLOT(abortMoveRequest()));
        connect(player, SIGNAL(doMove(const int&)), &board, SLOT(redPlayCol(const int&)));
        connect(player, SIGNAL(enableMouseInput()), &board, SLOT(enableMouse()));
        connect(player, SIGNAL(disableMouseInput()), &board, SLOT(disableMouse()));

        connect(player, SIGNAL(statusUpdate(const QString&)), this, SLOT(statusUpdate(const QString&)));
    }

    void GameWindow::connectYellow(Player* player)
    {
        if(yellowPlayer != 0) delete yellowPlayer;
        yellowPlayer = player;
        ui->labelP2Name->setText(yellowPlayer->name);

        board.setYellowHuman(player->isHuman());

        connect(&board, SIGNAL(yellowMoveRequest(const Board&)), player, SLOT(move(const Board&)));
        connect(&board, SIGNAL(yellowClickEvent(const GameBoard::MouseClickEvent&)), player, SLOT(mouseClick(const GameBoard::MouseClickEvent&)));
        connect(&board, SIGNAL(moveUndone()), player, SLOT(moveUndone()));
        connect(&board, SIGNAL(yellowAbortMoveRequest()), player, SLOT(abortMoveRequest()));
        connect(player, SIGNAL(doMove(const int&)), &board, SLOT(yellowPlayCol(const int&)));
        connect(player, SIGNAL(enableMouseInput()), &board, SLOT(enableMouse()));
        connect(player, SIGNAL(disableMouseInput()), &board, SLOT(disableMouse()));

        connect(player, SIGNAL(statusUpdate(const QString&)), this, SLOT(statusUpdate(const QString&)));
    }

// Private slots:
    void GameWindow::winner(const Piece& color)
    {
        if(color == Red)
            ui->labelWinnerLeft->setPixmap(QPixmap(":/sprites/winner.png"));
        else if(color == Yellow)
            ui->labelWinnerRight->setPixmap(QPixmap(":/sprites/winner.png"));
        else
            ui->labelDraw->setPixmap(QPixmap(":/sprites/draw.png"));
    }

    void GameWindow::clearWinner()
    {
        ui->labelWinnerLeft->clear();
        ui->labelWinnerRight->clear();
        ui->labelDraw->clear();
    }

    void GameWindow::enableUndoButton(const bool& enable)
    { ui->buttonUndoMove->setEnabled(enable); }

    void GameWindow::on_buttonUndoMove_clicked()
    {
        board.undoMove();
    }

    void GameWindow::statusUpdate(const QString& msg)
    {
        ui->statusBar->showMessage(msg);
    }

    void GameWindow::on_buttonBackToMenu_clicked()
    {
        // Abort all move requests
        redPlayer->abortMoveRequest();
        yellowPlayer->abortMoveRequest();
        backToMenu();
    }
