/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2008 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#include <QDragEnterEvent>
#include <QDropEvent>
#include <ksavefile.h>

#include <QHBoxLayout>

#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>
#include <KAction>
#include <KConfigDialog>
#include <KGameThemeSelector>

#include <KCmdLineArgs>
#include <KAboutData>

#include <kmessagebox.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <krun.h>

//#include "print.h" //TODO PORT
//#include "exportdlg.h"
#include "ksview.h"
#include "gameactions.h"
#include "renderer.h"

#include "puzzle.h" // TODO
#include "serializer.h"

#include <ktar.h>
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
		KMessageBox::information(this, i18n("Sorry, your solution contains mistakes.\n\nEnable \"Show errors\" in the settings to highlight them."));
		return;
	}

	QString msg;
	int secs = QTime(0,0).secsTo(required);
	int mins = secs / 60;
	secs = secs % 60;

	if(withHelp)
		if (mins == 0)
			msg = i18np("Congratulations! You made it in 1 second. With some tricks.", "Congratulations! You made it in %1 seconds. With some tricks.", secs);
		else if (secs == 0)
			msg = i18np("Congratulations! You made it in 1 minute. With some tricks.", "Congratulations! You made it in %1 minutes. With some tricks.", mins);
		else
			msg = i18nc("The two parameters are strings like '2 minutes' or '1 second'.", "Congratulations! You made it in %1 and %2. With some tricks.", i18np("1 minute", "%1 minutes", mins), i18np("1 second", "%1 seconds", secs));
	else
		if (mins == 0)
			msg = i18np("Congratulations! You made it in 1 second.", "Congratulations! You made it in %1 seconds.", secs);
		else if (secs == 0)
			msg = i18np("Congratulations! You made it in 1 minute.", "Congratulations! You made it in %1 minutes.", mins);
		else
			msg = i18nc("The two parameters are strings like '2 minutes' or '1 second'.", "Congratulations! You made it in %1 and %2.", i18np("1 minute", "%1 minutes", mins), i18np("1 second", "%1 seconds", secs));

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
	: KXmlGuiWindow(), m_gameVariants(new GameVariantCollection(this, true))
{
	setObjectName("ksudoku");

	m_gameWidget = 0;
	m_gameUI = 0;

	m_gameActions = 0;

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
	m_valueListWidget->setFixedWidth(60);

	m_welcomeScreen = new WelcomeScreen(wrapper, m_gameVariants);
	wrapper->layout()->addWidget(m_welcomeScreen);
	connect(m_welcomeScreen, SIGNAL(newGameStarted(const ::ksudoku::Game&,GameVariant*)), this, SLOT(startGame(const ::ksudoku::Game&)));
	showWelcomeScreen();

	// Register the gamevariants resource
	KGlobal::dirs()->addResourceType ("gamevariant", "data", KCmdLineArgs::aboutData()->appName());

	updateShapesList();


	QTimer *timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), this, SLOT(updateStatusBar()) );
	updateStatusBar();
	timer->start( 1000); //TODO PORT, false ); // 2 seconds single-shot timer
}

KSudoku::~KSudoku()
{
	endCurrentGame();
}

void KSudoku::updateShapesList()
{
	// TODO clear the list
	GameVariant* variant = 0;

	variant = new SudokuGame(i18n("Sudoku Standard (9x9)"), 9, m_gameVariants);
	variant->setDescription(i18n("The classic and fashionable game"));
	variant->setIcon("ksudoku-ksudoku_9x9");
	variant = new SudokuGame(i18n("Sudoku 16x16"), 16, m_gameVariants);
	variant->setDescription(i18n("Sudoku with 16 symbols"));
	variant->setIcon("ksudoku-ksudoku_16x16");
	variant = new SudokuGame(i18n("Sudoku 25x25"), 25, m_gameVariants);
	variant->setDescription(i18n("Sudoku with 25 symbols"));
	variant->setIcon("ksudoku-ksudoku_25x25");
#ifdef OPENGL_SUPPORT
	variant = new RoxdokuGame(i18n("Roxdoku 9 (3x3x3)"), 9, m_gameVariants);
	variant->setDescription(i18n("The Rox 3D sudoku"));
	variant->setIcon("ksudoku-roxdoku_3x3x3");
	variant = new RoxdokuGame(i18n("Roxdoku 16 (4x4x4)"), 16, m_gameVariants);
	variant->setDescription(i18n("The Rox 3D sudoku with 16 symbols"));
	variant->setIcon("ksudoku-roxdoku_4x4x4");
	variant = new RoxdokuGame(i18n("Roxdoku 25 (5x5x5)"), 25, m_gameVariants);
	variant->setDescription(i18n("The Rox 3D sudoku with 25 symbols"));
	variant->setIcon("ksudoku-roxdoku_5x5x5");
#endif

    QStringList filepaths = KGlobal::dirs()->findAllResources("gamevariant", "*.desktop", KStandardDirs::NoDuplicates); // Find files.

	QString variantName;
	QString variantDescr;
	QString variantDataPath;
	QString variantIcon;

	foreach(const QString &filepath, filepaths) {
		KConfig variantConfig(filepath, KConfig::SimpleConfig);
		KConfigGroup group = variantConfig.group ("KSudokuVariant");

		variantName = group.readEntry("Name", i18n("Missing Variant Name")); // Translated.
		variantDescr = group.readEntry("Description", ""); // Translated.
		variantIcon = group.readEntry("Icon", "ksudoku-ksudoku_9x9");
		variantDataPath = group.readEntry("FileName", "");
		if(variantDataPath == "") continue;
		variantDataPath = filepath.left(filepath.lastIndexOf("/")+1) + variantDataPath;

		variant = new CustomGame(variantName, variantDataPath, m_gameVariants);
		variant->setDescription(variantDescr);
		variant->setIcon(variantIcon);
	}
}

void KSudoku::startGame(const Game& game) {
	m_welcomeScreen->hide();
	endCurrentGame();
	
	
	KsView* view = new KsView(game, m_gameActions, this);

	view->setValueListWidget(m_valueListWidget);
	view->createView();

	connect(view, SIGNAL(valueSelected(int)), m_valueListWidget, SLOT(selectValue(int)));
	connect(m_valueListWidget, SIGNAL(valueSelected(int)), view, SLOT(selectValue(int)));
// 	connect(view, SIGNAL(valueSelected(int)), SLOT(updateStatusBar()));

	QWidget* widget = view->widget();
	m_gameUI = view;

	wrapper->layout()->addWidget(widget);
	widget->show();

	connect(game.interface(), SIGNAL(completed(bool,const QTime&,bool)), SLOT(onCompleted(bool,const QTime&,bool)));
	connect(game.interface(), SIGNAL(modified(bool)), SLOT(onModified(bool)));

	adaptActions2View();

	QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	policy.setHorizontalStretch(1);
	policy.setVerticalStretch(1);
	widget->setSizePolicy(policy);

	m_valueListWidget->setMaxValue(view->game().order());
	m_valueListWidget->selectValue(1);
	m_valueListWidget->show();
}

void KSudoku::endCurrentGame() {
	m_valueListWidget->hide();
	
	delete m_gameUI;
	m_gameUI = 0;
	
	adaptActions2View();

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

void KSudoku::showWelcomeScreen() {
	endCurrentGame();

	m_welcomeScreen->show();
}

void KSudoku::homepage()
{
	KRun::runUrl (KUrl("http://ksudoku.sourceforge.net/"), "text/html", this);
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
		KMessageBox::information(this, i18n("Sorry, no solutions have been found."));
		delete puzzle;
		return;
	} else if(state == 1) {
		KMessageBox::information(this, i18n("The Puzzle you entered has only one solution."));
	} else {
		KMessageBox::information(this, i18n("The Puzzle you entered has multiple solutions."));
	}

	if(KMessageBox::questionYesNo(this, i18n("Do you want to play the puzzle now?"), i18n("Play Puzzle"), KGuiItem(i18n("Play")), KStandardGuiItem::cancel() ) == KMessageBox::Yes)
	{
		startGame(ksudoku::Game(puzzle));
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

void KSudoku::setupActions()
{
	m_gameActions = new ksudoku::GameActions(actionCollection());
	m_gameActions->init();

	KShortcut shortcut;

	setAcceptDrops(true);

	KStandardGameAction::gameNew(this, SLOT(gameNew()), actionCollection());
	KStandardGameAction::load(this, SLOT(gameOpen()), actionCollection());
	m_gameSave = KStandardGameAction::save(this, SLOT(gameSave()), actionCollection());
	m_gameSaveAs = KStandardGameAction::saveAs(this, SLOT(gameSaveAs()), actionCollection());
	// TODO Print and Export are disabled due to missing port to KDE4
// 	KStandardGameAction::print(this, SLOT(gamePrint()), actionCollection());
	KStandardGameAction::quit(this, SLOT(close()), actionCollection());
	// TODO Print and Export are disables due to missing port to KDE4
// 	createAction("game_export", SLOT(gameExport()), i18n("Export"));

	KStandardAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

	//History
	KStandardGameAction::undo(this, SLOT(undo()), actionCollection());
	KStandardGameAction::redo(this, SLOT(redo()), actionCollection());

	KStandardGameAction::hint(this, SLOT(giveHint()), actionCollection());
	KStandardGameAction::solve(this, SLOT(autoSolve()), actionCollection());

	KAction* a = new KAction(this);
	actionCollection()->addAction("move_dub_puzzle", a);
	a->setText(i18n("Check"));
	a->setIcon(KIcon("games-endturn"));
	connect(a, SIGNAL(triggered(bool)), SLOT(dubPuzzle()));
	addAction(a);

	//WEB
	a = new KAction(this);
	actionCollection()->addAction("home_page", a);
	a->setText(i18n("Home Page"));
	a->setIcon(KIcon("internet-web-browser"));
	connect(a, SIGNAL(triggered(bool)), SLOT(homepage()));
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

void KSudoku::gameNew()
{
    // this slot is called whenever the Game->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

	if(!currentView()) return;

	// TODO onyl show this when there is a game running
	if(KMessageBox::questionYesNo(this, 
	                              i18n("Do you really want to end this game in order to start a new one?"),
	                              i18nc("window title", "Restart Game"),
	                              KGuiItem(i18nc("button label", "Restart Game")),
	                              KStandardGuiItem::cancel() ) != KMessageBox::Yes)
		return;

	showWelcomeScreen();
}

void KSudoku::gameOpen()
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

void KSudoku::gameSave()
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

void KSudoku::gameSaveAs()
{
    // this slot is called whenever the Game->Save As menu is selected,
	Game game = currentGame();
	if(!game.isValid()) return;

	game.setUrl(KFileDialog::getSaveUrl());
    if (!game.getUrl().isEmpty() && game.getUrl().isValid())
    	gameSave();
}


void KSudoku::gamePrint()
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

void KSudoku::gameExport()
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
	dialog->addPage(gameConfig, i18nc("Game Section in Config", "Game"), "games-config-options");
	dialog->addPage(new KGameThemeSelector(dialog, Settings::self(), KGameThemeSelector::NewStuffDisableDownload), i18n("Theme"), "games-config-theme");

    dialog->setHelp(QString(),"ksudoku");
	connect(dialog, SIGNAL(settingsChanged(const QString&)), SLOT(updateSettings()));
	dialog->show();
}

void KSudoku::updateSettings() {
	Renderer::instance()->loadTheme(Settings::theme());

	KsView* view = currentView();
	if(view) {
		int order = view->game().order();
		m_valueListWidget->setMaxValue(order);

		view->settingsChanged();
	}

	emit settingsChanged();
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
