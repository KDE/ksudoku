// part of KSUDOKU - by Francesco Rossi <redsh@email.it> 2005

#include "ksudoku.h"

#include <qpainter.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>
#include <ksavefile.h>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <KLocale>
#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KStandardAction>
#include <KToggleAction>

#include <QDomDocument>
#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kstatusbar.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kconfig.h>
//#include <kactionclasses.h>
#include <krun.h>
#include <kurlrequesterdialog.h>

#include "gameopt.h"
//#include "print.h" //TODO PORT
//#include "exportdlg.h"
#include "puzzlefactory.h"
#include "ksview.h"

#include <kstandardshortcut.h>

#include "puzzle.h" // TODO
#include "serializer.h"
#include <ktabwidget.h>

#include <ktar.h>
#include <qdir.h>
#include <kstandarddirs.h>
#include <kio/job.h>


// bool guidedMode;

void KSudoku::onCompleted(bool isCorrect, const QTime& required, bool withHelp) {
	if(!isCorrect) {
		KMessageBox::information(this, i18n("Sorry the solution you entered is not correct.\nIf you want to see error check Options->Guided mode please."));
		return;
	}
	
	QString msg;
 	int secs = QTime(0,0).secsTo(required);
 	int mins = secs / 60;
 	secs = secs % 60;

	if(withHelp)
		msg = i18n("Congratulations! You made it in %1 minutes and %2 seconds. With some tricks.", mins, secs);
	else
		msg = i18n("Congratulations!!!! You made it in %1 minutes and %2 seconds.", mins, secs);

	KMessageBox::information(this, msg);
	
}

void KSudoku::updateStatusBar()
{
	QString m="";
// 	QWidget* current = m_tabs->currentPage();
// 	if(KsView* view = dynamic_cast<KsView*>(current))
// 		m = view->status();
	if(currentView())
		m = currentView()->status();

	statusBar()->showMessage(m);
}

// KSudoku::KSudoku(ksudoku::Game* game)
// 	: KMainWindow(0,"ksudoku"), m_tabs(0)
// {
// 	m_tabs = new KTabWidget(this);
// 	m_tabs->setHoverCloseButton(true);
// 	m_tabs->show();
// 	setCentralWidget(m_tabs);
// 	
// 	readProperties( KApplication::kApplication()->config());
// 	setupActions();
// 	
// 	statusBar()->show();
// 	
// 	setupGUI();
// 	
// 	addGame(game);
// 	
// 	QTimer *timer = new QTimer( this );
//     connect( timer, SIGNAL(timeout()), this, SLOT(updateStatusBar()) );
//     timer->start( 1000, false ); // 2 seconds single-shot timer	
// }

KSudoku::KSudoku()
	: KXmlGuiWindow()
// 	, m_tabs(0)
	, m_autoDelCentralWidget(false)
{
	setObjectName("ksudoku");
// 	m_tabs = new KTabWidget(this);
// 	m_tabs->setHoverCloseButton(true);
// 	m_tabs->show();
// 	m_tabs->hide();
// 	setCentralWidget(m_tabs);
	
	// then, setup our actions
	readProperties( KGlobal::config().data());
	setupActions();

	// and a status bar
	statusBar()->show();
	setupGUI();

	// Apply the create the main window and ask the mainwindow to
	// automatically save settings if changed: window size, toolbar
	// position, icon size, etc.  Also to add actions for the statusbar
	// toolbar, and keybindings if necessary.
//	QGridLayout* top_layout = new QGridLayout(this);


// 	newGame();

// 	connect(this, SIGNAL(currentChanged(QWidget*)), this, SLOT(adaptActions2View(QWidget*)));

	// Setup Welcome Screen
// 	m_tabs->insertTab(new QLabel("Welcome to KSudoku (FAKE WELCOME)", this), "Welcome");
// <<<<<<< .mine
	wrapper = new QWidget();
	(void) new QVBoxLayout(wrapper);
	
	activeWidget = 0;
	QMainWindow::setCentralWidget(wrapper);
	wrapper->show();

	m_gameSelDlg = new GameSelectionDialog(0);	
// 	m_tabs->insertTab(m_gameSelDlg, "Welcome");
// 	m_gameSelDlg->show();
// 	setCentralWidget(m_gameSelDlg);
// =======
// 	gameSelDlg = new GameSelectionDialog(this);
// 	m_tabs->insertTab(gameSelDlg, "Welcome");
// >>>>>>> .r110
	QString title = i18n("Start a Game");
	m_gameSelDlg->addEntry("play-sudoku", i18n("Sudoku"), title);
	m_gameSelDlg->addEntry("play-roxdoku", i18n("Roxdoku (3D)"), title);

	//KGlobal::dirs()->addResourceType("shapes", KStandardDirs::kde_default("data") + QString::fromLatin1("ksudoku/"));
	updateCustomShapesList();

	title = i18n("Create your own Game");
	m_gameSelDlg->addEntry("edit-sudoku", i18n("Sudoku"), title);
	m_gameSelDlg->addEntry("edit-roxdoku", i18n("Roxdoku (3D)"), title);
// 	gameSelDlg->addEntry("edit-sudoku", i18n("Sudoku"), title);
// 	gameSelDlg->addEntry("edit-roxdoku", i18n("Roxdoku (3D)"), title);
	title = i18n("MORE GAMES");
	m_gameSelDlg->addEntry("shape-download", i18n("Download new shapes"), title);
	m_gameSelDlg->addEntry("shape-load", i18n("Load shapes manually"), title);
// 	m_gameSelDlg->showOptions();
	connect( m_gameSelDlg, SIGNAL( gameSelected(const QString&) ), this, SLOT( dlgSelectedGame(const QString&) ));
	connect(m_gameSelDlg, SIGNAL(gameSelected(const QString&)), this, SLOT(selectGameType(const QString&)));

	setCentralWidget(m_gameSelDlg, false);
	
	QTimer *timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), this, SLOT(updateStatusBar()) );
	updateStatusBar();
	timer->start( 1000); //TODO PORT, false ); // 2 seconds single-shot timer
}

//This is only a testing stub
QString KSudoku::getShapeName(QString path)
{
	int left  = path.lastIndexOf('/');
	int right = path.lastIndexOf('.');
	int len = right - left;
	return path.mid(left+1, len-1);
}

void KSudoku::updateCustomShapesList()
{
	QString title = i18n( "Start a Game" );
	QStringList mdirs = KGlobal::dirs()->findDirs("data", "ksudoku/");//, KStandardDirs::NoDuplicates);

	if ( mdirs.isEmpty() ) return;
	QStringList files;

	for (int i = 0; i < mdirs.size(); ++i)
	{
	        kDebug(11000) << "dir " << mdirs.at(i) << endl;
		QDir dir(mdirs.at(i));
		//printf("%s\n", (mdirs.at(i)).toLatin1());
		QStringList temp;
		temp += dir.entryList(QDir::Files, QDir::Name );
		for(int j=0; j < temp.size(); j++)
			if(temp.at(j) != QString( "ksudokuui.rc"))files += dir.absoluteFilePath( temp.at(j) );
	}

	for (int i = 0; i < files.size(); ++i)
	{
		if( !m_shapes.contains(getShapeName(files.at(i))) )
		{
			QString err;
			KUrl Url;
			Url.setPath(files.at(i));
			//SKSolver* sol = ksudoku::GraphCustom::createCustomSolver((*it));
			SKSolver* sol = ksudoku::Serializer::loadCustomShape(Url, this, 0);

			if(sol != NULL)
			{
				kDebug(11000) << "insert " << files.at(i) << endl;
				m_shapes.insert(getShapeName(files.at(i)),  sol);
				m_gameSelDlg->addEntry( "custom-"+getShapeName(files.at(i)), getShapeName(files.at(i)), title );
			}
			else
			{
				//printf("Error: file %s is not a valid shape file.\n", (files.at(i)).toLatin1());
				//TODO Error Message?
			}
		}
	}
	emit m_gameSelDlg->UPDATE();
	
}

KSudoku::~KSudoku()
{
}

void KSudoku::addGame(const Game& game) {
	GameType type = game.puzzle()->gameType(); //game solver()->g->sizeZ() > 1) ? 1 : 0;
	KsView* view = 0;

	switch(type){
		case sudoku: { //cUrly braces needed to avoid "crosses initialization" compile error
			ksudokuView* v = new ksudokuView(this, game, false);
// 			v->setup(game);
			connect( v, SIGNAL(changedSelectedNum()), this, SLOT(updateStatusBar()) );
			view = v;
			break;     }
		case roxdoku: {
			view = new RoxdokuView(game, this, "ksudoku-3dwnd");
			break;      }
		case custom:{
// 			SKPuzzle* puzzle = game.puzzle()->puzzle();
			//GraphCustom* gc = game.puzzle()->solver()->g;
			ksudokuView* v = new ksudokuView(this, game, true);
// 			v->setup(game);
			connect( v, SIGNAL(changedSelectedNum()), this, SLOT(updateStatusBar()) );
			view = v;
		}
		default:
			///@todo if here, BUG => throw exception (??)
			break;
	}
// 	m_tabs->insertTab(view, "Test");
// 	m_tabs->showPage(view);
	setCentralWidget(view->widget(), true);
}

void KSudoku::dlgSelectedGame(const QString& /*name*/)
{
// 	int order =  m_gameSelDlg->order;
// 	int difficulty = m_gameSelDlg->difficulty;
// 	int symmetry = m_gameSelDlg->symmetry;
// 
// 	if( name == QString("play-sudoku") )
// 	{
// 		stateChanged("dubbing", StateReverse);
// 		Game game = Game(PuzzleFactory().create_instance(sudoku, order, difficulty, symmetry));
// 		addGame(game);
// 	}
// 	else if( name == QString("play-roxdoku") )
// 	{
// 		stateChanged("dubbing", StateReverse);
// 		Game game = Game(PuzzleFactory().create_instance(roxdoku, order, difficulty, 1));
// 		addGame(game);
// 	}
// 	else if( name == QString("edit-sudoku") )
// 	{
// 		stateChanged("dubbing");
// 		Game game(PuzzleFactory().create_instance(sudoku, order, 0, 0, true));
// 		addGame(game);
// 	}
// 	else if( name == QString("edit-roxdoku") )
// 	{
// 		stateChanged("dubbing");
// 		Game game(PuzzleFactory().create_instance(sudoku, order, 0, 0, true));
// 		addGame(game);
// 	}
// 	else if( name == QString("shape-download") )
// 	{
// // 		KSudokuNewStuff* mNewStuff = new KSudokuNewStuff( this );
// // 		mNewStuff->download();
// 	}
// 	else if( name == QString("shape-load") )
// 	{
// // 		loadCustomShapeFromPath();
// 	}
// 	else
// 	{
// 		int l = name.find('-');
// 		if(l==-1) return; //error
// 		QString shape = name.right(name.length()-l-1);
// 
// 		if(m_shapes.contains(shape))
// 		{
// 			if(m_shapes[shape] != NULL)
// 			{
// 				stateChanged("dubbing", StateReverse);
// 				Game game(PuzzleFactory().create_instance(custom, 0, difficulty, 1, false, m_shapes[shape]));
// 				addGame(game);
// 			}
// 			else
// 			{
// 				//TODO error
// 				printf("error1\n");
// 				return;
// 			}
// 		}
// 		else
// 		{
// 			//TODO error
// 			printf("error2\n");
// 			return;
// 		}
// 
// 		
// 	}
}

// TODO will be deprecated
void KSudoku::newGame() {
	selectGameType(m_defaultAction);
}

void KSudoku::loadGame(const KUrl& Url) {
	QString errorMsg;
	Game game = ksudoku::Serializer::load(Url, this, &errorMsg);
	if(!game.isValid()) {
		KMessageBox::information(this, errorMsg);
		return;
	}
	
	addGame(game);
}

void KSudoku::setCentralWidget(QWidget* widget, bool autoDel) {
	QWidget* oldWidget = activeWidget;
	if(oldWidget) oldWidget->hide();
	if(m_autoDelCentralWidget) delete oldWidget; //moving up here fixes a roxdoku window bug
	m_autoDelCentralWidget = autoDel;
	
	widget->show();
	wrapper->layout()->addWidget(widget);
	activeWidget = widget;

	readProperties(KGlobal::config().data()); //correct order: otherwise settings are not loaded
	adaptActions2View();
}

void KSudoku::showWelcomeScreen() {
	m_gameOptionsDlg = 0;
	setCentralWidget(m_gameSelDlg, false);
}

void KSudoku::selectGameType(const QString& type) {
	// TODO do this in an integrated dialog
	int typev = 0;
	int order = 0;
	bool noSymmetry = false;
	bool dub = false;
	QString shapeName;
	
	if(type == "play-sudoku") {
		typev = 0;
	} else if(type == "play-roxdoku") {
		typev = 1;
		noSymmetry = true;
	} else if(type == "edit-sudoku") {
		typev = 0;
		dub = true;
	} else if(type == "edit-roxdoku") {
		typev = 1;
		dub = true;
	} else if(type == "shape-download") {
#if 0
		KSudokuNewStuff* mNewStuff = new KSudokuNewStuff( this );
		mNewStuff->download();
#endif
		return;
	} else if(type == "shape-load") {
		loadCustomShapeFromPath();
		return;
	} else if(type.startsWith("custom-")) {
		shapeName = type.mid(QString("custom-").length());
		if(m_shapes.contains(shapeName) && m_shapes[shapeName]) {
			typev = 2;
			order = -1;
			noSymmetry = true;
		} else {
			shapeName =  QString();
		}
	} else {
		return;
	}
	
	m_defaultAction = type;
	
	QWidget *page = new QWidget;
	QVBoxLayout *layout = new QVBoxLayout;
	page->setLayout(layout);
     
	GameOptionsDialog* options = new GameOptionsDialog(page, dub, typev);
	options->setShapeName(shapeName);
	if(noSymmetry) options->setSymmetry(-1);
	if(order != 0) options->setOrder(order);
	m_gameOptionsDlg = options;
	QPushButton* btnBack = new QPushButton(i18n("To Welcomescreen"), this);
	QPushButton* btnStart = 0;
	if(dub) {
		btnStart = new QPushButton(i18n("Edit Game"), this);
		m_optionEnterOwnGame = true;
	} else {
		btnStart = new QPushButton(i18n("Start Game"), this);
		m_optionEnterOwnGame = false;
	}
	QHBoxLayout* buttonsl = new QHBoxLayout;
	layout->addLayout(buttonsl);

	buttonsl->addWidget(btnBack);
	buttonsl->addWidget(btnStart);

	connect(btnBack, SIGNAL(clicked()), this, SLOT(showWelcomeScreen()));
	connect(btnStart, SIGNAL(clicked()), this, SLOT(startSelectedGame()));

	setCentralWidget(page, true);
}

void KSudoku::startSelectedGame() {
	uint order = m_gameOptionsDlg->order();
	int difficulty = m_gameOptionsDlg->difficulty();
	uint type = m_gameOptionsDlg->type();
	uint symmetry = m_gameOptionsDlg->symmetry();
	QString shapeName = m_gameOptionsDlg->shapeName();

	if(m_optionEnterOwnGame) {
		stateChanged("dubbing", StateReverse);
		if(type == 0) {
			Game game(PuzzleFactory().create_instance(sudoku, order, 0, 0, true));
			addGame(game);
		} else if(type == 1) {
			Game game(PuzzleFactory().create_instance(roxdoku, order, 0, 0, true));
			addGame(game);
		}
	} else {
		stateChanged("dubbing");
		if(type == 0) {
			Game game(PuzzleFactory().create_instance(sudoku, order, difficulty, symmetry));
			addGame(game);
		} else if(type == 1) {
			Game game(PuzzleFactory().create_instance(roxdoku, order, difficulty, SIMMETRY_NONE));
			addGame(game);
		} else if(type == 2) {
			Game game(PuzzleFactory().create_instance(custom, 0, difficulty, 1, false, m_shapes[shapeName]));
			addGame(game);
		}
	}
}

void KSudoku::mouseOnlySuperscript()
{
// 	QWidget* current = m_tabs->currentPage();
//	QWidget* current = (QWidget*)currentView();
	if(ksudokuView* view = dynamic_cast<ksudokuView*>(currentView()))
		view->mouseOnlySuperscript = !view->mouseOnlySuperscript;
	else return;

	saveProperties( KGlobal::config().data());
}

void KSudoku::setGuidedMode()
{
// 	QWidget* current = m_tabs->currentPage();
//	QWidget* current = (QWidget*) currentView();
	
	if(KsView* view = dynamic_cast<ksudokuView*>(currentView())) {
		((ksudokuView*)view)->toggleGuided();
		((ksudokuView*)view)->update();
		
	//	/// TODO fix this (rerender through reinit); ???
	//	view->setGame(view->game());
	}
	else
		return;

	saveProperties( KGlobal::config().data());
}

void KSudoku::homepage()
{
	KRun::runUrl (KUrl("http://ksudoku.sourceforge.net/"), "text/html", this);
}
void KSudoku::support()
{
	KRun::runUrl (KUrl("http://sourceforge.net/project/project_donations.php?group_id=147876"), "text/html", this);
}
void KSudoku::sendComment()
{
	KRun::runUrl (KUrl("http://ksudoku.sourceforge.net/newcomment.php"), "text/html", this);
}

void KSudoku::checkForUpdates()
{
	QString name;
	QString version;
	QString myVer = "0.3";
	KIO::NetAccess::download(KUrl("http://ksudoku.sourceforge.net/latest.php"), name, this);

	QFile file(name);
	if (!file.open(QIODevice::ReadOnly)) 
	{
		KMessageBox::information(this, "Could not get the response from server.");
		return;
	}

	QByteArray tmpbuf = file.readLine();
	QString buf = QString::fromAscii(tmpbuf);
	file.close();
	if(buf == myVer)
		KMessageBox::information(this, "Your program is at the latest version");
	else
	{
		QString msg = i18n("Your program version is %1, the latest version is %2.\nDo you want to update?", myVer, buf);
		if(KMessageBox::questionYesNo(this, msg) == KMessageBox::Yes)
			KRun::runUrl (KUrl("http://ksudoku.sourceforge.net/3.htm"), "text/html", this);
	}
	KIO::NetAccess::removeTempFile( name );
}

void KSudoku::giveHint()
{
	Game game = currentGame();
	if(!game.isValid()) return;
	game.giveHint();
}

void KSudoku::autoSolve()
{
	Game game = currentGame();
	if(!game.isValid()) return;
	game.autoSolve();
}
	
void KSudoku::dubPuzzle()
{
	Game game = currentGame();
	
	if(!game.isValid()) return;
	
	if(!game.simpleCheck()) {
		KMessageBox::information(this, i18n("The puzzle you entered contains some errors."));
		return;
	}

	int forks = 0;
	ksudoku::Puzzle* puzzle = game.puzzle()->dubPuzzle();
	int state = puzzle->init(game.allValues(), &forks);
		
	if(state <= 0) {
		KMessageBox::information(this, i18n("Sorry, No solutions have been found."));
		delete puzzle;
		return;
	} else if(state == 1) {
		KMessageBox::information(this, i18n("The Puzzle you entered has only one solution. (Forks required: %1)",forks));
	} else {
		KMessageBox::information(this, i18n("The Puzzle you entered has multiple solutions."));
	}
	
	if(KMessageBox::questionYesNo(this, i18n("Do you want to play the puzzle now?")) == 3)
	{
		ksudoku::Game* newGame = new ksudoku::Game(puzzle);
			
	// 		(new KSudoku(newGame))->show();
		addGame(*newGame);
		delete newGame;
	}
	else
	{
		delete puzzle;
	}
	
	return;
}
	
void KSudoku::genMultiple()
{
	//KMessageBox::information(this, i18n("Sorry, this feature is under development."));
}

void KSudoku::selectNumber(uint value) {
// 	QWidget* current = m_tabs->currentPage()<
	if(ksudokuView* view = dynamic_cast<ksudokuView*>( currentView())) {
		view->current_selected_number = value;
	} else if (RoxdokuView* view = dynamic_cast<RoxdokuView*>( currentView())) {
		view->selected_number = value;
	/*} else if (ksudokuCustomView* view = dynamic_cast<ksudokuCustomView*>(current)) {
		view->current_selected_number = value;*/
	} else return;
	updateStatusBar();
}

void KSudoku::set0 () { selectNumber(0); }
void KSudoku::set1 () { selectNumber(1); }
void KSudoku::set2 () { selectNumber(2); }
void KSudoku::set3 () { selectNumber(3); }
void KSudoku::set4 () { selectNumber(4); }
void KSudoku::set5 () { selectNumber(5); }
void KSudoku::set6 () { selectNumber(6); }
void KSudoku::set7 () { selectNumber(7); }
void KSudoku::set8 () { selectNumber(8); }
void KSudoku::set9 () { selectNumber(9); }
void KSudoku::set10 () { selectNumber(10); }
void KSudoku::set11 () { selectNumber(11); }
void KSudoku::set12 () { selectNumber(12); }
void KSudoku::set13 () { selectNumber(13); }
void KSudoku::set14 () { selectNumber(14); }
void KSudoku::set15 () { selectNumber(15); }
void KSudoku::set16 () { selectNumber(16); }

void KSudoku::set17 () { selectNumber(17); }
void KSudoku::set18 () { selectNumber(18); }
void KSudoku::set19 () { selectNumber(19); }
void KSudoku::set20 () { selectNumber(20); }
void KSudoku::set21 () { selectNumber(21); }
void KSudoku::set22 () { selectNumber(22); }
void KSudoku::set23 () { selectNumber(23); }
void KSudoku::set24 () { selectNumber(24); }
void KSudoku::set25 () { selectNumber(25); }

void KSudoku::KDE3Action(QString text, QWidget* object, const char* slot, QString name)
{
	KAction* a = new KAction(text, object);
	connect(a, SIGNAL(triggered(bool)),object, slot);
	actionCollection()->addAction(name, a);
}

void KSudoku::setupActions()
{
	setAcceptDrops(true);
	
	KStandardAction::openNew(this, SLOT(fileNew()),    actionCollection());
	KStandardAction::open   (this, SLOT(fileOpen()),   actionCollection());
	KStandardAction::save   (this, SLOT(fileSave()),   actionCollection());
	KStandardAction::saveAs (this, SLOT(fileSaveAs()), actionCollection());
	KStandardAction::print  (this, SLOT(filePrint()),  actionCollection());
	KStandardAction::quit   (kapp, SLOT(quit()),       actionCollection());

	KStandardAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

	KDE3Action(i18n("&Export"), this, SLOT(fileExport()), "file_export");
	
	KDE3Action(i18n("del"), this, SLOT(set0()), "del");
	KDE3Action(i18n("1"), this, SLOT(set1 ()), "1");
	KDE3Action(i18n("2"), this, SLOT(set2 ()), "2");
	KDE3Action(i18n("3"), this, SLOT(set3 ()), "3");
	KDE3Action(i18n("4"), this, SLOT(set4 ()), "4");
	KDE3Action(i18n("5"), this, SLOT(set5 ()), "5");
	KDE3Action(i18n("6"), this, SLOT(set6 ()), "6");
	KDE3Action(i18n("7"), this, SLOT(set7 ()), "7");
	KDE3Action(i18n("8"), this, SLOT(set8 ()), "8");
	KDE3Action(i18n("9"), this, SLOT(set9 ()), "9");
	KDE3Action(i18n("j"), this, SLOT(set10()), "j");
	KDE3Action(i18n("k"), this, SLOT(set11()), "k");
	KDE3Action(i18n("l"), this, SLOT(set12()), "l");
	KDE3Action(i18n("m"), this, SLOT(set13()), "m");
	KDE3Action(i18n("n"), this, SLOT(set14()), "n");
	KDE3Action(i18n("o"), this, SLOT(set15()), "o");
	KDE3Action(i18n("p"), this, SLOT(set16()), "p");

	KDE3Action(i18n("q"), this, SLOT(set17()), "q");
	KDE3Action(i18n("r"), this, SLOT(set18()), "r");
	KDE3Action(i18n("s"), this, SLOT(set19()), "s");
	KDE3Action(i18n("t"), this, SLOT(set20()), "t");
	KDE3Action(i18n("u"), this, SLOT(set21()), "u");
	KDE3Action(i18n("v"), this, SLOT(set22()), "v");
	KDE3Action(i18n("w"), this, SLOT(set23()), "w");
	KDE3Action(i18n("y"), this, SLOT(set24()), "x");
	KDE3Action(i18n("x"), this, SLOT(set25()), "y");

	KDE3Action(i18n("a"), this, SLOT(set1()), "a");
	KDE3Action(i18n("b"), this, SLOT(set2()), "b");
	KDE3Action(i18n("c"), this, SLOT(set3()), "c");
	KDE3Action(i18n("d"), this, SLOT(set4()), "d");
	KDE3Action(i18n("e"), this, SLOT(set5()), "e");
	KDE3Action(i18n("f"), this, SLOT(set6()), "f");
	KDE3Action(i18n("g"), this, SLOT(set7()), "g");
	KDE3Action(i18n("h"), this, SLOT(set8()), "h");
	KDE3Action(i18n("i"), this, SLOT(set9()), "i");

	//History
	KStandardAction::undo   (this, SLOT(undo()),actionCollection());//, "move_undo");
	KStandardAction::redo   (this, SLOT(redo()),actionCollection());//, "move_redo");

	KAction* aa = new KAction(i18n("Push checkpoint (add)"), this);
	aa->setShortcut( Qt::CTRL+Qt::Key_A);
	connect(aa, SIGNAL(triggered(bool)),this, SLOT(push()));
	actionCollection()->addAction("move_add_group",aa);
	
	aa = new KAction(i18n("Pop checkpoint (extract)"), this);
	aa->setShortcut( Qt::CTRL+Qt::Key_E);
	connect(aa, SIGNAL(triggered(bool)),this, SLOT(pop()));
	actionCollection()->addAction("move_undo_group",aa);

	// TODO replace this with KStdGameAction members when having libkdegames
	KDE3Action(i18n("Give Hint!"), this, SLOT(giveHint()), "move_hint"); //DONE
	KDE3Action(i18n("Solve!"), this, SLOT(autoSolve()), "move_solve");	//DONE
	//(void)new KAction(i18n("Generate Multiple"), 0, this, SLOT(genMultiple()), actionCollection(), "genMultiple");
	KDE3Action(i18n("Check"),  this, SLOT(dubPuzzle()), "move_dub_puzzle");

	//WEB
	KDE3Action(i18n("Check for updates"), this, SLOT(checkForUpdates()), "checkForUpdates");
	KDE3Action(i18n("Home page"), this, SLOT(homepage()), "Home_page");
	KDE3Action(i18n("Support this project"), this, SLOT(support()), "support");
	KDE3Action(i18n("Send comment"), this, SLOT(sendComment()), "SendComment");

	//Settings
	KToggleAction* a;
  	a = new KToggleAction(i18n("Mouse-Only superscript mode"),this);
	connect(a, SIGNAL(triggered(bool)),this,  SLOT(mouseOnlySuperscript()));
	actionCollection()->addAction("mouseOnlySuperscript", a);
	a->setChecked(false);
	a->setEnabled(false);	
	a=new KToggleAction(i18n("Guided mode (mark wrong red)"),  this);
	connect(a, SIGNAL(triggered(bool)),this,  SLOT(setGuidedMode()));
	actionCollection()->addAction("guidedMode", a);
	a->setChecked(false);
	a->setEnabled(false);

	a=new KToggleAction(i18n("Show tracker"), 0);
	connect(a, SIGNAL(triggered(bool)),this,  SLOT(setShowTracker()));
	actionCollection()->addAction("showTracker", a);
	a->setChecked(true);
	a->setEnabled(false);
}

void KSudoku::adaptActions2View() {
	// TODO This whole function is only a temporary hack, views should have their own UI
	
	
	if(ksudokuView* view = dynamic_cast<ksudokuView*>(currentView())) {
		KToggleAction* a;
		if((a = dynamic_cast<KToggleAction*>(action("mouseOnlySuperscript")))) {
			a->setEnabled(true);
			a->setChecked(view->mouseOnlySuperscript);
		}
		if((a = dynamic_cast<KToggleAction*>(action("guidedMode")))) {
			a->setEnabled(true);
			a->setChecked(view->guidedMode());
		}
		if((a = dynamic_cast<KToggleAction*>(action("showTracker")))) {
			a->setEnabled(true);
			a->setChecked(view->showTracker);
		}
	} else if(RoxdokuView* view = dynamic_cast<RoxdokuView*>(currentView())) {
		KToggleAction* a;
		if((a = dynamic_cast<KToggleAction*>(action("mouseOnlySuperscript")))) {
			a->setEnabled(false);
			a->setChecked(false);
		}
		if((a = dynamic_cast<KToggleAction*>(action("guidedMode")))) {
			a->setEnabled(true);
			a->setChecked(view->guidedMode());
		}
		if((a = dynamic_cast<KToggleAction*>(action("showTracker")))) {
			a->setEnabled(false);
			a->setChecked(false);
		}
	} else {
		KToggleAction* a;
		if((a = dynamic_cast<KToggleAction*>(action("mouseOnlySuperscript")))) {
			a->setEnabled(false);
			a->setChecked(false);
		}
		if((a = dynamic_cast<KToggleAction*>(action("guidedMode")))) {
			a->setEnabled(false);
			a->setChecked(false);
		}
		if((a = dynamic_cast<KToggleAction*>(action("showTracker")))) {
			a->setEnabled(false);
			a->setChecked(false);
		}
	}
	
	Game game = currentGame();
	if(game.isValid()) {
		action("file_save")->setEnabled(true);
		action("file_save_as")->setEnabled(true);
		
		action("edit_undo")->setEnabled(game.canUndo());
		action("edit_undo")->setEnabled(game.canRedo());
		action("move_add_group")->setEnabled(game.canAddCheckpoint());
		action("move_undo_group")->setEnabled(game.canUndo2Checkpoint());
		
		action("move_hint")      ->setEnabled(   game.puzzle()->hasSolution());
		action("move_solve")     ->setEnabled(   game.puzzle()->hasSolution());
		action("move_dub_puzzle")->setEnabled( ! game.puzzle()->hasSolution());
	} else {
		action("file_save")->setEnabled(false);
		action("file_save_as")->setEnabled(false);
		action("edit_undo")->setEnabled(false);
		action("edit_redo")->setEnabled(false);
		action("move_add_group")->setEnabled(false);
		action("move_undo_group")->setEnabled(false);
	
		action("move_hint")->setEnabled(false);
		action("move_solve")->setEnabled(false);
		action("move_dub_puzzle")->setEnabled(false);
	}
}

void KSudoku::onModified(bool /*isModified*/) {
	Game game = currentGame();
	if(game.isValid()) {
		action("edit_undo")->setEnabled(game.canUndo());
		action("edit_redo")->setEnabled(game.canRedo());
		action("move_add_group")->setEnabled(game.canAddCheckpoint());
		action("move_undo_group")->setEnabled(game.canUndo2Checkpoint());
	}
}

void KSudoku::undo() {
	Game game = currentGame();
	if(!game.isValid()) return;
	
	game.interface()->undo();
	
	if(!game.canUndo()) {
		action("edit_undo")->setEnabled(false);
	}
}

void KSudoku::redo() {
	Game game = currentGame();
	if(!game.isValid()) return;
	
	game.interface()->redo();
	
	if(!game.canRedo()) {
		action("edit_redo")->setEnabled(false);
	}
}

void KSudoku::push()
{
	// TODO replace this with history
// 	if(type == 0) {if(m_view) m_view->push();return;}
// 	if(glwin) glwin->push();
}

void KSudoku::pop()
{
	// TODO replace this with history
// 	if(type == 0) {if(m_view)  m_view->pop(); return;}
// 	if(glwin) glwin->pop();
}

void KSudoku::setShowTracker()
{
// 	QWidget* current = m_tabs->currentPage();
//	QWidget* current = (QWidget*) currentView();
	if(ksudokuView* view = dynamic_cast<ksudokuView*>(currentView())) {
		view->showTracker = !view->showTracker;
	} else return;

	saveProperties((KConfig* )KGlobal::config().data());
}

void KSudoku::saveProperties(KConfig *config)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
	KConfigGroup group = config->group("ksudoku General");
	
	if(ksudokuView* view = dynamic_cast<ksudokuView*>(currentView())) {
		group.writeEntry("guidedMode", QVariant(view->guidedMode()));
		group.writeEntry("mouseOnlySuperscript",  QVariant(view->mouseOnlySuperscript));
		group.writeEntry("showTracker", QVariant(view->showTracker ));
	} else if(RoxdokuView* view = dynamic_cast<RoxdokuView*>(currentView())) {
		group.writeEntry("guidedMode", QVariant(view->guidedMode()));
	}

	config->sync();
}

void KSudoku::readProperties(KConfig *config)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'
	KConfigGroup group = config->group("ksudoku General");

    QString Url = group.readEntry("lastUrl", "");
	
	if(ksudokuView* view = dynamic_cast<ksudokuView*>(currentView())) {
		view->setGuidedMode(group.readEntry("guidedMode", true));
		view->showTracker = group.readEntry("showTracker", true);
   		view->mouseOnlySuperscript = group.readEntry("mouseOnlySuperscript", true);
	} else if(RoxdokuView* view = dynamic_cast<RoxdokuView*>(currentView())) {
		view->setGuidedMode(group.readEntry("guidedMode", true));
	}
}

void KSudoku::dragEnterEvent(QDragEnterEvent */*event*/)
{
    // accept uri drops only
	
	//TODO PORT
    //KUrl::List::fromMimeData( e->mimeData() )
	
    //event->accept(KUrlDrag::canDecode(event));
}

void KSudoku::dropEvent(QDropEvent *event)
{
	//TODO PORT
   KUrl::List Urls = KUrl::List::fromMimeData( event->mimeData() );
  

    if ( !Urls.isEmpty() ) 
    {
        // okay, we have a URI.. process it
        const KUrl &Url = Urls.first();
		
		Game game = ksudoku::Serializer::load(Url, this);
// 		if(game)
// 			(new KSudoku(game))->show();
		if(game.isValid())
			addGame(game);
// 		delete game;
    }
 
}

void KSudoku::fileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

	// TODO onyl show this when there is a game running
	if(KMessageBox::questionYesNo(this, i18n("Do you really want to end this game in order to start a new one")) != KMessageBox::Yes)
		return;
	
	newGame();
}

void KSudoku::fileOpen()
{
	// this slot is called whenever the File->Open menu is selected,
	// the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
	// button is clicked
	// standard filedialog
	KUrl Url = KFileDialog::getOpenUrl(KUrl(), QString::null, this, i18n("Open Location"));
	
	if (!Url.isEmpty() && Url.isValid())
	{
		Game game = ksudoku::Serializer::load(Url, this);
		if(!game.isValid()) {
			KMessageBox::error(this, i18n("Couldn't load game."));
			return;
		}
		
		game.setUrl(Url);
// 		(new KSudoku(game))->show();
		addGame(game);
// 		delete game;
	}	
}

void KSudoku::fileSave()
{
    // this slot is called whenever the File->Save menu is selected,
    // the Save shortcut is pressed (usually CTRL+S) or the Save toolbar
    // button is clicked

    // save the current file
	
	Game game = currentGame();
	if(!game.isValid()) return;

	if(game.getUrl().isEmpty()) game.setUrl(KFileDialog::getSaveUrl());
 	if (!game.getUrl().isEmpty() && game.getUrl().isValid())
		ksudoku::Serializer::store(game, game.getUrl(), this);
}

void KSudoku::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
	Game game = currentGame();
	if(!game.isValid()) return;
	
	game.setUrl(KFileDialog::getSaveUrl());
    if (!game.getUrl().isEmpty() && game.getUrl().isValid())
    	fileSave();
}


void KSudoku::filePrint()
{
    // this slot is called whenever the File->Print menu is selected,
    // the Print shortcut is pressed (usually CTRL+P) or the Print toolbar
    // button is clicked

// 	Game* game = currentGame();

//TODO PORT
// 	ksudoku::KsView* view = currentView();
//	if(view)
//		ksudoku::Print p(*view);// *game, game->solver()->g->sizeZ() > 0);
	//else ??? give message noting to print with hint what is printable ??
}

void KSudoku::fileExport()
{
	//TODO PORT
	/*
	Game game = currentGame();
	if(!game.isValid()) return;
	
	ksudoku::ExportDlg e(*game.puzzle(), *game.symbols() );

	e.exec();
	*/
}

void KSudoku::optionsPreferences()
{
    // popup some sort of preference dialog, here
/*    ksudokuPreferences dlg;
    if (dlg.exec())
    {
        // redo your settings
    }*/
}

void KSudoku::changeStatusbar(const QString& text)
{
    // display the text on the statusbar
    statusBar()->showMessage(text);
}

void KSudoku::changeCaption(const QString& text)
{
    // display the text on the caption
    setCaption(text);
}

Game KSudoku::currentGame() const {
	ksudoku::KsView* view = currentView();

	if(view)
		return view->game();
	else
		return Game();
}

ksudoku::KsView* KSudoku::currentView() const{
// 	if(ksudoku::KsView* view = dynamic_cast<KsView*>(m_tabs->currentPage()))
// 		return view;
// 	else
// 		return 0;
	
	// TODO this might cause trouble as the central widget don't have to be a
	// KsView instance
//	if(activeWidget)
	if(centralWidget() == 0) return 0;

	return dynamic_cast<KsView*>(activeWidget);
}

void KSudoku::loadCustomShapeFromPath()
{
	KUrl Url = KFileDialog::getOpenUrl( KUrl(), QString::null, this, i18n("Open Location") );
	
	if ( Url.isEmpty() || !Url.isValid() )
	{
		//TODO ERROR
		return;
	}	
	
	QString tmpFile;
// 	bool success = false;
	QDomDocument doc;
	if(!KIO::NetAccess::download( Url, tmpFile, this )) 
	{
		//TODO ERROR
		return;	
	}
	
	KStandardDirs myStdDir;
	const QString destDir = myStdDir.saveLocation( "data", /*kapp->instanceName() + TODO PORT */"ksudoku/", true );
	KStandardDirs::makeDir( destDir );

	KTar archive( tmpFile );

	if ( archive.open( QIODevice::ReadOnly ) )
	{
		const KArchiveDirectory *archiveDir = archive.directory();
		archiveDir->copyTo( destDir );
		archive.close();
	}
	else
	{
		//just copy
		KIO::file_copy (Url, destDir);
	}

	KIO::NetAccess::removeTempFile(tmpFile);
	updateCustomShapesList();
}

#if 0
KSudokuNewStuff::KSudokuNewStuff( KSudoku* v ) :
        KNewStuff( "ksudoku", (QWidget*) v )
{
	parent = v;
}

bool KSudokuNewStuff::install( const QString &fileName )
{
	KTar archive( fileName );
	if ( !archive.open( QIODevice::ReadOnly ) )
		return false;

	const KArchiveDirectory *archiveDir = archive.directory();
	KStandardDirs myStdDir;
	const QString destDir = myStdDir.saveLocation("data", /*kapp->instanceName() + TODO PORT*/"ksudoku/", true);
	KStandardDirs::makeDir(destDir);

	archiveDir->copyTo(destDir);
	archive.close();
	//find custom shapes
	parent->updateCustomShapesList();
	return true;
}

bool KSudokuNewStuff::createUploadFile( const QString &/*fileName*/ )
{
	return true;
}
#endif



#include "ksudoku.moc"
