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

#ifndef DIALOGABOUT_H
#define DIALOGABOUT_H

#include <QDialog>

namespace Ui { class DialogAbout; }

class DialogAbout : public QDialog
{
    Q_OBJECT
    
    public:
        explicit DialogAbout(QWidget *parent = 0);
        ~DialogAbout();

    private slots:
        void on_buttonAboutQt_clicked();

        void on_buttonClose_clicked();

    private:
        Ui::DialogAbout *ui;
};

#endif // DIALOGABOUT_H