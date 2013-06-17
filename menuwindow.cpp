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

#include "menuwindow.h"
#include "ui_menuwindow.h"

#include "humanplayer.h"
#include "dumbplayer.h"
#include "chanceplayer.h"
#include "perfectplayer.h"

// Public:
    MenuWindow::MenuWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MenuWindow), dlgAbout(0)
    {
        ui->setupUi(this);

        connect(ui->buttonQuit, SIGNAL(clicked()), this, SLOT(close()));
    }

    MenuWindow::~MenuWindow()
    {
        if(dlgAbout != 0) delete dlgAbout;
        delete ui;
    }

// Public slots:
    void MenuWindow::show()
    {
        QMainWindow::show();
        setFixedSize(size());
    }

// Private slots:
    void MenuWindow::on_buttonAbout_clicked()
    {
        if(dlgAbout == 0)
            dlgAbout = new DialogAbout(this);
        dlgAbout->show();
    }

    void MenuWindow::on_buttonStartGame_clicked()
    {
        const int p1Type = ui->comboP1Type->currentIndex();
        const int p2Type = ui->comboP2Type->currentIndex();

        Player* p1 = 0;
        Player* p2 = 0;

        if(p1Type == 0)         p1 = new HumanPlayer(tr("Mens"), true);
        else if(p1Type == 1)    p1 = new DumbPlayer(tr("PC Dom"), true);
        else if(p1Type == 2)    p1 = new ChancePlayer(tr("PC Makkelijk"), true, 1, 4);
        else if(p1Type == 3)    p1 = new ChancePlayer(tr("PC Gemiddeld"), true, 3, 5);
        else if(p1Type == 4)    p1 = new ChancePlayer(tr("PC Moeilijk"), true, 5, 6);
        else if(p1Type == 5)    p1 = new PerfectPlayer(tr("PC Perfect"), true);

        if(p2Type == 0)         p2 = new HumanPlayer(tr("Mens"), false);
        else if(p2Type == 1)    p2 = new DumbPlayer(tr("PC Dom"), false);
        else if(p2Type == 2)    p2 = new ChancePlayer(tr("PC Makkelijk"), false, 1, 4);
        else if(p2Type == 3)    p2 = new ChancePlayer(tr("PC Gemiddeld"), false, 3, 5);
        else if(p2Type == 4)    p2 = new ChancePlayer(tr("PC Moeilijk"), false, 5, 6);
        else if(p2Type == 5)    p2 = new PerfectPlayer(tr("PC Perfect"), false);

        hide();
        startGame(p1, p2);
    }
