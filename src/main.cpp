/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2007 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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
#include <kapplication.h>
//#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kconfigdialogmanager.h>
#include "sudoku_solver.h"
#include "serializer.h"

#include <cstdlib>
#include <time.h>


static const char description[] =
    I18N_NOOP("KSudoku - Sudokus and more");

static const char version[] = "0.5";


int main(int argc, char **argv)
{
	std::srand(time(0));

	KAboutData about("ksudoku", 0,
	                 ki18n("KSudoku"),
	                 version,
	                 ki18n("KSudoku - Sudoku games and more"),
	                 KAboutData::License_GPL_V2,
	                 ki18n("(c) 2005-2007 The KSudoku Authors"),
	                 KLocalizedString(), "http://ksudoku.sourceforge.net/");
	about.addAuthor( ki18n("Francesco Rossi"), ki18n("KSudoku Author"), "redsh@email.it" );
	about.addAuthor( ki18n("Johannes Bergmeier"), ki18n("Maintainer"), "Johannes.Bergmeier@gmx.net" );
	about.addAuthor( ki18n("Mick Kappenburg"), ki18n("Printing and export of 0.4"), "ksudoku@kappendburg.net");
	about.addAuthor( ki18n("Thanks to NeHe for OpenGL tutorials"), KLocalizedString(), "nehe.gamedev.net");
	KCmdLineArgs::init(argc, argv, &about);

	KCmdLineOptions options;
	options.add("+[URL]", ki18n( "Document to open" ));
	KCmdLineArgs::addCmdLineOptions(options);
	KApplication app;
	KGlobal::locale()->insertCatalog("libkdegames");

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
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		if (args->count() != 0)
		{
			for (int i = 0; i < args->count(); ++i)
			{
				widget->loadGame(args->url(i));
			}
		}
		args->clear();
	//} //TODO PORT

	return app.exec();
}
