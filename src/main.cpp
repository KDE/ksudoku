// part of KSUDOKU - by Francesco Rossi <redsh@email.it> 2005

#include "ksudoku.h"
#include <kapplication.h>
//#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include "sudoku_solver.h"
#include "serializer.h"

#include <cstdlib>
#include <time.h>

static const char description[] =
    I18N_NOOP("A KDE Application");

static const char version[] = "0.3";

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP( "Document to open" ), 0 },
    KCmdLineLastOption
};


int main(int argc, char **argv)
{
	std::srand(time(0));

	KAboutData about("ksudoku", I18N_NOOP("ksudoku"), version, description
	                 , KAboutData::License_GPL_V2, "(C) 2005 Francesco Rossi"
	                 , 0, 0, "redsh@email.it");
	about.addAuthor( "Francesco Rossi", 0, "redsh@email.it" );
	about.addAuthor( "Thanks to NeHe for opengl tutorials", 0, "nehe.gamedev.net");
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(options);
	KApplication app;

	// register ourselves as a dcop client
//	app.dcopClient()->registerAs(app.name(), false); //TODO PORT

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
