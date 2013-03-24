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

#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QMainWindow>
#include "dialogabout.h"
#include "player.h"

namespace Ui { class MenuWindow; }

class MenuWindow : public QMainWindow
{
    Q_OBJECT
        
    public:
        explicit MenuWindow(QWidget *parent = 0);
        ~MenuWindow();

    public slots:
        void show();

    signals:
        void startGame(Player* p1, Player* p2);
        
    private slots:
        void on_buttonAbout_clicked();

        void on_buttonStartGame_clicked();

private:
            Ui::MenuWindow *ui;
            DialogAbout* dlgAbout;
};

#endif // MENUWINDOW_H
