/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/


#include <QApplication>
#include <QDataStream>
#include <QString>

#include <KAboutData>
#include <KLocalizedString>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // get our DCOP client and attach so that we may use it
    DCOPClient *client = app.dcopClient();
    client->attach();

    // do a 'send' for now
    BoardContents data;
    QDataStream ds(data, QIODevice::WriteOnly);
    if (argc > 1)
        ds << QString(argv[1]);
    else
        ds << QString("https://www.kde.org");
    client->send("ksudoku", "ksudokuIface", "openURL(QString)", data);

    return app.exec();
}
