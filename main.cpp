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

#include <QtGui/QApplication>
#include <QFile>
#include <ctime>
#include "menuwindow.h"
#include "gamewindow.h"

int main(int argc, char *argv[])
{
    // Create the application
    QApplication app(argc, argv);

    // Load and apply the stylesheet
    QFile stylesheet(":/style/style.css");
    if(stylesheet.open(QFile::ReadOnly))
    {
        app.setStyleSheet(stylesheet.readAll());
        stylesheet.close();
    }

    // Create the windows
    GameWindow gameWindow;
    MenuWindow menuWindow;

    // Conntect the menu window and the game window
    QObject::connect(&menuWindow, SIGNAL(startGame(Player*, Player*)), &gameWindow, SLOT(startGame(Player*,Player*)));
    QObject::connect(&gameWindow, SIGNAL(backToMenu()), &menuWindow, SLOT(show()));

    // Show the menu window
    menuWindow.show();
    
    // Execute the application
    return app.exec();
}
