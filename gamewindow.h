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

#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include "gameboard.h"
#include "player.h"

namespace Ui { class GameWindow; }

class GameWindow : public QMainWindow
{
    Q_OBJECT
    
    public:
        explicit GameWindow(QWidget* parent = 0);
        ~GameWindow();

    public slots:
        void startGame(Player* red, Player* yellow);

    signals:
        void backToMenu();

    private:
        Ui::GameWindow* ui;
        GameBoard board;

        Player* redPlayer;
        Player* yellowPlayer;

        void connectRed(Player* player);
        void connectYellow(Player* player);

    private slots:
        void winner(const Piece& color);
        void clearWinner();
        void enableUndoButton(const bool& enable);
        void statusUpdate(const QString& msg);

        void on_buttonUndoMove_clicked();
        void on_buttonBackToMenu_clicked();
};

#endif // GAMEWINDOW_H
