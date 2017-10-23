/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2007 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
 *   Copyright 2015      Ian Wadham <iandw.au@gmail.com>                   *
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

#include "ksudoku.h"

#include <KAboutData>
#include <KCrash>
#include <KLocalizedString>
#include <KConfigDialogManager>

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QUrl>

#include <cstdlib>
#include <ctime>


static const char description[] =
    I18N_NOOP("KSudoku - Sudokus and more");

static const char version[] = "1.4";


int main(int argc, char **argv)
{
	qsrand(std::time(nullptr));
	QApplication app(argc, argv);
	KLocalizedString::setApplicationDomain("ksudoku");

	KAboutData about("ksudoku",
	                 i18n("KSudoku"),
	                 version,
	                 i18n(description),
	                 KAboutLicense::GPL_V2,
	                 i18n("(c) 2005-2007 The KSudoku Authors"),
	                 QString(), "https://games.kde.org/game.php?game=ksudoku");
	about.addAuthor( i18n("Francesco Rossi"), i18n("KSudoku Author"), "redsh@email.it" );
	about.addAuthor( i18n("Johannes Bergmeier"), i18n("Maintainer"), "Johannes.Bergmeier@gmx.net" );
	about.addAuthor( i18n("Ian Wadham"), i18n("New puzzle generator and solver"), "iandw.au@gmail.com" );
	about.addAuthor( i18n("Mick Kappenburg"), i18n("Printing and export of 0.4"), "ksudoku@kappendburg.net");
	about.addAuthor( i18n("Thanks to NeHe for OpenGL tutorials"), QString(), "nehe.gamedev.net");
	about.addCredit( i18n("David Bau"), i18n("Algorithms for new puzzle generator and solver at davidbau.com/archives/2006/09/04/sudoku_generator.html"), "");

	KAboutData::setApplicationData(about);
	app.setOrganizationDomain(QStringLiteral("kde.org"));
	app.setWindowIcon(QIcon::fromTheme(QStringLiteral("ksudoku")));

	QCommandLineParser parser;
	about.setupCommandLine(&parser);
	parser.addPositionalArgument(QLatin1String("[URL]"), i18n( "Document to open" ));

	parser.process(app);
	about.processCommandLine(&parser);

	KCrash::initialize();

	// register ourselves as a dcop client
//	app.dcopClient()->registerAs(app.name(), false); //TODO PORT

	KConfigDialogManager::changedMap()->insert("ksudoku::SymbolConfigListWidget", SIGNAL(itemChanged(QListWidgetItem*)));

	// see if we are starting with session management
	/*if (app.isRestored())
	{
			RESTORE(KSudoku);
	}
	else
	{*/
		KSudoku *widget = new KSudoku;
		widget->show();

		// no session.. just start up normally
		if (parser.positionalArguments().count() != 0)
		{
			for (int i = 0; i < parser.positionalArguments().count(); ++i)
			{
				widget->loadGame(QUrl::fromUserInput(parser.positionalArguments().at(i), QDir::currentPath()));
			}
		}

	//} //TODO PORT

	return app.exec();
}
