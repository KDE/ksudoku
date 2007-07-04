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
#include <KConfigDialog>

#include <KCmdLineArgs>
#include <KAboutData>

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
#include <kstandardgameaction.h>

#include "gamevariants.h"
#include "welcomescreen.h"
#include "valuelistwidget.h"

#include "settings.h"
#include "config.h"


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
// 	if(currentView())
// 		m = currentView()->status();

	// TODO fix this: add new status bar generation code

	statusBar()->showMessage(m);
}

KSudoku::KSudoku()
	: KXmlGuiWindow(), m_gameVariants(new GameVariantCollection(this, true)), m_autoDelCentralWidget(false)
{
	setObjectName("ksudoku");

	m_gameWidget = 0;
	m_gameUI = 0;
	activeWidget = 0;

	m_selectValueMapper = new QSignalMapper(this);
	connect(m_selectValueMapper, SIGNAL(mapped(int)), this, SLOT(selectValue(int)));
	m_enterValueMapper = new QSignalMapper(this);
	connect(m_enterValueMapper, SIGNAL(mapped(int)), this, SLOT(enterValue(int)));
	m_markValueMapper = new QSignalMapper(this);
	connect(m_markValueMapper, SIGNAL(mapped(int)), this, SLOT(markValue(int)));

	// then, setup our actions
	setupActions();

	// and a status bar
	statusBar()->show();
	setupGUI();

	wrapper = new QWidget();
	(void) new QHBoxLayout(wrapper);
	QMainWindow::setCentralWidget(wrapper);
	wrapper->show();

	// Create ValueListWidget
	m_valueListWidget = new ValueListWidget(wrapper);
	wrapper->layout()->addWidget(m_valueListWidget);
// 	m_valueListWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
	m_valueListWidget->setFixedWidth(60);

	m_welcomeScreen = new WelcomeScreen(wrapper, m_gameVariants);
	connect(m_welcomeScreen, SIGNAL(newGameStarted(const Game&,GameVariant*)), this, SLOT(startGame(const Game&)));
	showWelcomeScreen();

	// Register the gamevariants resource
    KGlobal::dirs()->addResourceType ("gamevariant", "data",
		KCmdLineArgs::aboutData()->appName());

	updateShapesList();

	m_symbols.setEnabledTables(Settings::symbols());


	QTimer *timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), this, SLOT(updateStatusBar()) );
	updateStatusBar();
	timer->start( 1000); //TODO PORT, false ); // 2 seconds single-shot timer
}

void KSudoku::updateShapesList()
{
	// TODO clear the list

	(void) new SudokuGame(i18n("Sudoku Standard (9x9)"), 9, m_gameVariants);
	(void) new SudokuGame(i18n("Sudoku 16x16"), 16, m_gameVariants);
	(void) new SudokuGame(i18n("Sudoku 25x25"), 25, m_gameVariants);
	(void) new RoxdokuGame(i18n("Roxdoku 9 (3x3x3)"), 9, m_gameVariants);
	(void) new RoxdokuGame(i18n("Roxdoku 16 (4x4x5)"), 16, m_gameVariants);
	(void) new RoxdokuGame(i18n("Roxdoku 25 (5x5x5)"), 25, m_gameVariants);

    QStringList filepaths = KGlobal::dirs()->findAllResources("gamevariant", "*.desktop", KStandardDirs::NoDuplicates); // Find files.

	QString variantName;
	QString variantDescr;
	QString variantDataPath;

    foreach(QString filepath, filepaths) {
		KConfig variantConfig(filepath, KConfig::OnlyLocal);
		KConfigGroup group = variantConfig.group ("KSudokuVariant");

		variantName = group.readEntry("Name", i18n("Missing Variant Name")); // Translated.
		variantDescr = group.readEntry("Description", ""); // Translated.
		variantDataPath = group.readEntry("FileName", "");
		if(variantDataPath == "") continue;
		variantDataPath = filepath.left(filepath.lastIndexOf("/")+1) + variantDataPath;

		(void) new CustomGame(variantName, variantDataPath, m_gameVariants);
	}
}

KSudoku::~KSudoku()
{
}

void KSudoku::startGame(const Game& game) {
	KsView* view = new KsView(game, this);
	
	view->setValueListWidget(m_valueListWidget);
	view->createView();	
	view->setSymbolTable(m_symbols.selectTable(view->game().order()));
	
	connect(view, SIGNAL(valueSelected(int)), m_valueListWidget, SLOT(selectValue(int)));
	connect(m_valueListWidget, SIGNAL(valueSelected(int)), view, SLOT(selectValue(int)));
	connect(view, SIGNAL(valueSelected(int)), SLOT(updateStatusBar()));


	m_welcomeScreen->hide();

	QWidget* widget = view->widget();
	m_gameUI = view;

	wrapper->layout()->addWidget(widget);
	
	Game game2(game);
	connect(game2.interface(), SIGNAL(completed(bool,const QTime&,bool)), SLOT(onCompleted(bool,const QTime&,bool)));
	connect(game2.interface(), SIGNAL(modified(bool)), SLOT(onModified(bool)));

	widget->show();
	wrapper->layout()->addWidget(widget);
	activeWidget = widget;
	m_autoDelCentralWidget = true;
	adaptActions2View();

	QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	policy.setHorizontalStretch(1);
	policy.setVerticalStretch(1);
	widget->setSizePolicy(policy);

	widget->addAction(m_moveUpAct);
	widget->addAction(m_moveDownAct);
	widget->addAction(m_moveLeftAct);
	widget->addAction(m_moveRightAct);

	m_valueListWidget->setCurrentTable(m_symbols.selectTable(view->game().	order()), view->game().order());
	m_valueListWidget->selectValue(1);
	m_valueListWidget->show();
}

void KSudoku::loadGame(const KUrl& Url) {
	QString errorMsg;
	Game game = ksudoku::Serializer::load(Url, this, &errorMsg);
	if(!game.isValid()) {
		KMessageBox::information(this, errorMsg);
		return;
	}

	startGame(game);
}

void KSudoku::setCentralWidget(QWidget* widget, bool autoDel) {
	QWidget* oldWidget = activeWidget;
	if(oldWidget) oldWidget->hide();
	if(m_autoDelCentralWidget) delete oldWidget; //moving up here fixes a roxdoku window bug
	m_autoDelCentralWidget = autoDel;

	widget->show();
	wrapper->layout()->addWidget(widget);
	activeWidget = widget;

	adaptActions2View();
}

void KSudoku::showWelcomeScreen() {
	m_valueListWidget->hide();
	delete m_gameUI;
	m_gameUI = 0;

	setCentralWidget(m_welcomeScreen, false);
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
		startGame(*newGame);
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

void KSudoku::selectValue(int value) {
	KsView* view = currentView();
	if(!view) return;

	view->selectValue(value);
	updateStatusBar();
}

void KSudoku::enterValue(int value) {
	KsView* view = currentView();
	if(!view) return;

	view->enterValue(value);
}

void KSudoku::markValue(int value) {
	KsView* view = currentView();
	if(!view) return;

	view->markValue(value);
}

void KSudoku::moveUp() {
	KsView* view = currentView();
	if(!view) return;

	view->moveUp();
}

void KSudoku::moveDown() {
	KsView* view = currentView();
	if(!view) return;

	view->moveDown();
}

void KSudoku::moveLeft() {
	KsView* view = currentView();
	if(!view) return;

	view->moveLeft();
}

void KSudoku::moveRight() {
	KsView* view = currentView();
	if(!view) return;

	view->moveRight();
}

void KSudoku::clearCell() {
	KsView* view = currentView();
	if(!view) return;

	view->enterValue(0);
}

void KSudoku::KDE3Action(const QString& text, QWidget* object, const char* slot, const QString& name)
{
	KAction* a = new KAction(text, object);
	connect(a, SIGNAL(triggered(bool)),object, slot);
	actionCollection()->addAction(name, a);
}

void KSudoku::setupActions()
{
	KShortcut shortcut;

	setAcceptDrops(true);

	KStandardGameAction::gameNew(this, SLOT(fileNew()), actionCollection());
	KStandardGameAction::load(this, SLOT(fileOpen()), actionCollection());
	m_gameSave = KStandardGameAction::save(this, SLOT(fileSave()), actionCollection());
	m_gameSaveAs = KStandardGameAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
	KStandardGameAction::print(this, SLOT(filePrint()), actionCollection());
	KStandardGameAction::quit(this, SLOT(quit()), actionCollection());

	KStandardAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

	QAction* gameExport = actionCollection()->addAction("game_export");
	gameExport->setText(i18n("&Export"));
	connect(gameExport, SIGNAL(triggered(bool)), SLOT(fileExport()));


	for(int i = 0; i < 25; ++i) {
		KAction* a = new KAction(this);
		actionCollection()->addAction(QString("val-select%1").arg(i+1,2,10,QChar('0')), a);
		a->setText(i18n("Select %1 (%2)", i+1, QChar('a'+i)));
		m_selectValueMapper->setMapping(a, i+1);
		connect(a, SIGNAL(triggered(bool)), m_selectValueMapper, SLOT(map()));
		addAction(a);

		a = new KAction(this);
		actionCollection()->addAction(QString("val-enter%1").arg(i+1,2,10,QChar('0')), a);
		a->setText(i18n("Enter %1 (%2)", i+1, QChar('a'+i)));
		shortcut = a->shortcut();
		shortcut.setPrimary( Qt::Key_A + i);
		if(i < 9) {
			shortcut.setAlternate( Qt::Key_1 + i);
		}
		a->setShortcut(shortcut);
		m_enterValueMapper->setMapping(a, i+1);
		connect(a, SIGNAL(triggered(bool)), m_enterValueMapper, SLOT(map()));
		addAction(a);

		a = new KAction(this);
		actionCollection()->addAction(QString("val-mark%1").arg(i+1,2,10,QChar('0')), a);
		a->setText(i18n("Mark %1 (%2)", i+1, QChar('a'+i)));
		shortcut = a->shortcut();
		shortcut.setPrimary( Qt::ShiftModifier | Qt::Key_A + i);
		if(i < 9) {
			shortcut.setAlternate( Qt::ShiftModifier | Qt::Key_1 + i);
		}
		a->setShortcut(shortcut);
		m_markValueMapper->setMapping(a, i+1);
		connect(a, SIGNAL(triggered(bool)), m_markValueMapper, SLOT(map()));
		addAction(a);
	}

	m_moveUpAct = actionCollection()->addAction("move_up");
	m_moveUpAct->setText(i18n("Move Up"));
	m_moveUpAct->setShortcut(Qt::Key_Up);
	connect(m_moveUpAct, SIGNAL(triggered(bool)), SLOT(moveUp()));
// 	addAction(moveUpAct);

	m_moveDownAct = actionCollection()->addAction("move_down");
	m_moveDownAct->setText(i18n("Move Down"));
	m_moveDownAct->setShortcut(Qt::Key_Down);
	connect(m_moveDownAct, SIGNAL(triggered(bool)), SLOT(moveDown()));
// 	addAction(moveDownAct);

	m_moveLeftAct = actionCollection()->addAction("move_left");
	m_moveLeftAct->setText(i18n("Move Left"));
	m_moveLeftAct->setShortcut(Qt::Key_Left);
	connect(m_moveLeftAct, SIGNAL(triggered(bool)), SLOT(moveLeft()));
// 	addAction(moveLeftAct);

	m_moveRightAct = actionCollection()->addAction("move_right");
	m_moveRightAct->setText(i18n("Move Right"));
	m_moveRightAct->setShortcut(Qt::Key_Right);
	connect(m_moveRightAct, SIGNAL(triggered(bool)), SLOT(moveRight()));
// 	addAction(moveRightAct);

	KAction* clearCellAct = new KAction(this);
	actionCollection()->addAction("move_clear_cell", clearCellAct);
	clearCellAct->setText(i18n("Clear Cell"));
	shortcut = clearCellAct->shortcut();
	shortcut.setPrimary(Qt::Key_Backspace);
	shortcut.setAlternate(Qt::Key_Delete);
	clearCellAct->setShortcut(shortcut);
	connect(clearCellAct, SIGNAL(triggered(bool)), SLOT(clearCell()));
	addAction(clearCellAct);

	//History
	QAction* undoAct = actionCollection()->addAction("move_undo");
	undoAct->setIcon(KIcon("edit-undo"));
	undoAct->setText(i18n("Undo"));
	connect(undoAct, SIGNAL(triggered(bool)), this, SLOT(undo()));

	QAction* redoAct = actionCollection()->addAction("move_redo");
	redoAct->setIcon(KIcon("edit-redo"));
	redoAct->setText(i18n("Redo"));
	connect(redoAct, SIGNAL(triggered(bool)), this, SLOT(redo()));

	// TODO replace this with KStdGameAction members when having libkdegames
	KDE3Action(i18n("Give Hint!"), this, SLOT(giveHint()), "move_hint"); //DONE
	KDE3Action(i18n("Solve!"), this, SLOT(autoSolve()), "move_solve");	//DONE
	//(void)new KAction(i18n("Generate Multiple"), 0, this, SLOT(genMultiple()), actionCollection(), "genMultiple");
	KDE3Action(i18n("Check"),  this, SLOT(dubPuzzle()), "move_dub_puzzle");

	//WEB
	KDE3Action(i18n("Home page"), this, SLOT(homepage()), "Home_page");
	KDE3Action(i18n("Support this project"), this, SLOT(support()), "support");
	KDE3Action(i18n("Send comment"), this, SLOT(sendComment()), "SendComment");
}

void KSudoku::adaptActions2View() {
	Game game = currentGame();

	m_gameSave->setEnabled(game.isValid());
	m_gameSaveAs->setEnabled(game.isValid());
	if(game.isValid()) {
		action("move_undo")->setEnabled(game.canUndo());
		action("move_undo")->setEnabled(game.canRedo());

		action("move_hint")      ->setEnabled(   game.puzzle()->hasSolution());
		action("move_solve")     ->setEnabled(   game.puzzle()->hasSolution());
		action("move_dub_puzzle")->setEnabled( ! game.puzzle()->hasSolution());
	} else {
		action("move_undo")->setEnabled(false);
		action("move_redo")->setEnabled(false);

		action("move_hint")->setEnabled(false);
		action("move_solve")->setEnabled(false);
		action("move_dub_puzzle")->setEnabled(false);
	}
}

void KSudoku::onModified(bool /*isModified*/) {
	Game game = currentGame();
	if(game.isValid()) {
		action("move_undo")->setEnabled(game.canUndo());
		action("move_redo")->setEnabled(game.canRedo());
	}
}

void KSudoku::undo() {
	Game game = currentGame();
	if(!game.isValid()) return;

	game.interface()->undo();

	if(!game.canUndo()) {
		action("move_undo")->setEnabled(false);
	}
}

void KSudoku::redo() {
	Game game = currentGame();
	if(!game.isValid()) return;

	game.interface()->redo();

	if(!game.canRedo()) {
		action("move_redo")->setEnabled(false);
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
			startGame(game);
// 		delete game;
    }

}

void KSudoku::fileNew()
{
    // this slot is called whenever the Game->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

	if(!currentView()) return;

	// TODO onyl show this when there is a game running
	if(KMessageBox::questionYesNo(this, i18n("Do you really want to end this game in order to start a new one")) != KMessageBox::Yes)
		return;

	showWelcomeScreen();
}

void KSudoku::fileOpen()
{
	// this slot is called whenever the Game->Open menu is selected,
	// the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
	// button is clicked
	// standard filedialog
	KUrl Url = KFileDialog::getOpenUrl(KUrl(), QString(), this, i18n("Open Location"));

	if (!Url.isEmpty() && Url.isValid())
	{
		Game game = ksudoku::Serializer::load(Url, this);
		if(!game.isValid()) {
			KMessageBox::error(this, i18n("Could not load game."));
			return;
		}

		game.setUrl(Url);
// 		(new KSudoku(game))->show();
		startGame(game);
// 		delete game;
	}
}

void KSudoku::fileSave()
{
    // this slot is called whenever the Game->Save menu is selected,
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
    // this slot is called whenever the Game->Save As menu is selected,
	Game game = currentGame();
	if(!game.isValid()) return;

	game.setUrl(KFileDialog::getSaveUrl());
    if (!game.getUrl().isEmpty() && game.getUrl().isValid())
    	fileSave();
}


void KSudoku::filePrint()
{
    // this slot is called whenever the Game->Print menu is selected,
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
	if ( KConfigDialog::showDialog("settings") ) return;

	KConfigDialog *dialog = new KConfigDialog(this, "settings", Settings::self());

	GameConfig* gameConfig = new GameConfig();
	dialog->addPage(gameConfig, i18nc("Game Section in Config", "Game"), "game");
			
	SymbolConfig* symbolConfig = new SymbolConfig(&m_symbols);
	dialog->addPage(symbolConfig, i18n("Symbol Themes"), "theme");
	
	connect(dialog, SIGNAL(settingsChanged(const QString&)), SLOT(settingsChanged()));
    dialog->show();
}

void KSudoku::settingsChanged() {
	m_symbols.setEnabledTables(Settings::symbols());

	KsView* view = currentView();
	if(view) {
		int order = view->game().order();
		SymbolTable* table = m_symbols.selectTable(order);
		view->setSymbolTable(table);
		m_valueListWidget->setCurrentTable(table, order);
		
		view->settingsChanged();
	}
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
// 	if(centralWidget() == 0) return 0;

// 	return dynamic_cast<KsView*>(activeWidget);

	return m_gameUI;
}

void KSudoku::loadCustomShapeFromPath()
{
	KUrl Url = KFileDialog::getOpenUrl( KUrl(), QString(), this, i18n("Open Location") );

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
	updateShapesList();
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
	parent->updateShapesList();
	return true;
}

bool KSudokuNewStuff::createUploadFile( const QString &/*fileName*/ )
{
	return true;
}
#endif



#include "ksudoku.moc"
