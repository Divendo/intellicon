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
        const QString p1Type = ui->comboP1Type->currentText();
        const QString p2Type = ui->comboP2Type->currentText();

        Player* p1 = 0;
        Player* p2 = 0;

        if(p1Type == "Menselijke speler")               p1 = new HumanPlayer("Mens", true);
        else if(p1Type == "Domme computer")             p1 = new DumbPlayer("PC Dom", true);
        else if(p1Type == "Makkelijke computer")        p1 = new ChancePlayer("PC Makkelijk", true, 1, 4);
        else if(p1Type == "Gemiddelde computer")        p1 = new ChancePlayer("PC Gemiddeld", true, 3, 5);
        else if(p1Type == "Moeilijke computer")         p1 = new ChancePlayer("PC Moeilijk", true, 5, 6);
        else if(p1Type == "Perfect spelende computer")  p1 = new PerfectPlayer("PC Perfect", true);

        if(p2Type == "Menselijke speler")               p2 = new HumanPlayer("Mens", false);
        else if(p2Type == "Domme computer")             p2 = new DumbPlayer("PC Dom", false);
        else if(p2Type == "Makkelijke computer")        p2 = new ChancePlayer("PC Makkelijk", false, 1, 4);
        else if(p2Type == "Gemiddelde computer")        p2 = new ChancePlayer("PC Gemiddeld", false, 3, 5);
        else if(p2Type == "Moeilijke computer")         p2 = new ChancePlayer("PC Moeilijk", false, 5, 6);
        else if(p2Type == "Perfect spelende computer")  p2 = new PerfectPlayer("PC Perfect", false);

        hide();
        startGame(p1, p2);
    }
