/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2008 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
 *   Copyright 2012      Ian Wadham <iandw.au@gmail.com>                   *
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

#include "globals.h"
#include "ksudoku.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <ksavefile.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>

#include <QPrinter>
#include <QPrintDialog>

#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>
#include <KAction>
#include <KConfigDialog>

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgamethemeselector.h>

#include <KCmdLineArgs>
#include <KAboutData>

#include <kmessagebox.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <krun.h>

#include "ksview.h"
#include "gameactions.h"
#include "renderer.h"

#include "puzzle.h" // TODO
#include "skgraph.h"
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

using namespace ksudoku;

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

// void KSudoku::updateStatusBar()
// {
// 	QString m="";
// // 	QWidget* current = m_tabs->currentPage();
// // 	if(KsView* view = dynamic_cast<KsView*>(current))
// // 		m = view->status();
// // 	if(currentView())
// // 		m = currentView()->status();
// 
// 	// TODO fix this: add new status bar generation code
// 
// 	statusBar()->showMessage(m);
// }

KSudoku::KSudoku()
	:
	KXmlGuiWindow(),
	m_gameVariants(new GameVariantCollection(this, true)),
	m_printer(0),
	m_p(0),
	m_quadrant(0)
{
	setObjectName( QLatin1String("ksudoku" ));

	m_gameWidget = 0;
	m_gameUI = 0;

	m_gameActions = 0;

	// then, setup our actions
	setupActions();

	setupGUI(ToolBar | Keys | Save | Create | StatusBar);

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
	connect(m_welcomeScreen, SIGNAL(newGameStarted(const::ksudoku::Game&,GameVariant*)), this, SLOT(startGame(const::ksudoku::Game&)));

	setupStatusBar(m_welcomeScreen->difficulty(),
		       m_welcomeScreen->symmetry());

	showWelcomeScreen();

	// Register the gamevariants resource
	KGlobal::dirs()->addResourceType ("gamevariant", "data", KCmdLineArgs::aboutData()->appName());

	updateShapesList();

// 	QTimer *timer = new QTimer( this );
// 	connect( timer, SIGNAL(timeout()), this, SLOT(updateStatusBar()) );
// 	updateStatusBar();
// 	timer->start( 1000); //TODO PORT, false ); // 2 seconds single-shot timer
}

KSudoku::~KSudoku()
{
	delete m_p;
	delete m_printer;
	endCurrentGame();
}

void KSudoku::updateShapesList()
{
	// TODO clear the list
	GameVariant* variant = 0;

	variant = new SudokuGame(i18n("Sudoku Standard (9x9)"), 9, m_gameVariants);
	variant->setDescription(i18n("The classic and fashionable game"));
	variant->setIcon("ksudoku-ksudoku_9x9");
#ifdef OPENGL_SUPPORT
	variant = new RoxdokuGame(i18n("Roxdoku 9 (3x3x3)"), 9, m_gameVariants);
	variant->setDescription(i18n("The Rox 3D Sudoku"));
	variant->setIcon("ksudoku-roxdoku_3x3x3");
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

	// Put variants first and extra sizes last.
	variant = new SudokuGame(i18n("Sudoku 16x16"), 16, m_gameVariants);
	variant->setDescription(i18n("Sudoku with 16 symbols"));
	variant->setIcon("ksudoku-ksudoku_16x16");
	variant = new SudokuGame(i18n("Sudoku 25x25"), 25, m_gameVariants);
	variant->setDescription(i18n("Sudoku with 25 symbols"));
	variant->setIcon("ksudoku-ksudoku_25x25");
#ifdef OPENGL_SUPPORT
	variant = new RoxdokuGame(i18n("Roxdoku 16 (4x4x4)"), 16, m_gameVariants);
	variant->setDescription(i18n("The Rox 3D sudoku with 16 symbols"));
	variant->setIcon("ksudoku-roxdoku_4x4x4");
	variant = new RoxdokuGame(i18n("Roxdoku 25 (5x5x5)"), 25, m_gameVariants);
	variant->setDescription(i18n("The Rox 3D sudoku with 25 symbols"));
	variant->setIcon("ksudoku-roxdoku_5x5x5");
#endif
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
	widget->setFocus();

	connect(game.interface(), SIGNAL(completed(bool,QTime,bool)), SLOT(onCompleted(bool,QTime,bool)));
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

// Check the game setup, copy the puzzle, init and solve the copy and show the
// result (i.e. implement the "Check" action). If the user agrees, start play.

void KSudoku::dubPuzzle()
{
	Game game = currentGame();

	if(!game.isValid()) return;

	if(!game.simpleCheck()) {
		KMessageBox::information(this, i18n("The puzzle you entered contains some errors."));
		return;
	}

	// Create a new Puzzle object, with same Graph and solution flag = true.
	ksudoku::Puzzle* puzzle = game.puzzle()->dubPuzzle();

	// Copy the given values of the puzzle, then run it through the solver.
	// The solution, if valid, is saved in puzzle->m_solution2.
	int state = puzzle->init(game.allValues());

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
 	KStandardGameAction::print(this, SLOT(gamePrint()), actionCollection());
	KStandardGameAction::quit(this, SLOT(close()), actionCollection());
	// TODO Export is disabled due to missing port to KDE4.
// 	createAction("game_export", SLOT(gameExport()), i18n("Export"));

	KStandardAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

	//History
	KStandardGameAction::undo(this, SLOT(undo()), actionCollection());
	KStandardGameAction::redo(this, SLOT(redo()), actionCollection());

	KStandardGameAction::hint(this, SLOT(giveHint()), actionCollection());
	KStandardGameAction::solve(this, SLOT(autoSolve()), actionCollection());

	KAction* a = new KAction(this);
	actionCollection()->addAction( QLatin1String( "move_dub_puzzle" ), a);
	a->setText(i18n("Check"));
	a->setIcon(KIcon( QLatin1String( "games-endturn" )));
	connect(a, SIGNAL(triggered(bool)), SLOT(dubPuzzle()));
	addAction(a);

	//WEB
	a = new KAction(this);
	actionCollection()->addAction( QLatin1String( "home_page" ), a);
	a->setText(i18n("Home Page"));
	a->setIcon(KIcon( QLatin1String( "internet-web-browser" )));
	connect(a, SIGNAL(triggered(bool)), SLOT(homepage()));
}

void KSudoku::setupStatusBar (int difficulty, int symmetry)
{
	// Use the standard combo box for difficulty, from KDE Games library.
	const int nStandardLevels = 4;
	const KGameDifficulty::standardLevel standardLevels[nStandardLevels] =
			{KGameDifficulty::VeryEasy, KGameDifficulty::Easy,
			 KGameDifficulty::Medium,   KGameDifficulty::Hard};

	statusBar()->addPermanentWidget (new QLabel (i18n("Difficulty")));
	KGameDifficulty::init (this, this,
		SLOT (difficultyChanged(KGameDifficulty::standardLevel)),
		SLOT (difficultyChanged(int)));
	KGameDifficulty::addStandardLevel(KGameDifficulty::VeryEasy);
	KGameDifficulty::addStandardLevel(KGameDifficulty::Easy);
	KGameDifficulty::addStandardLevel(KGameDifficulty::Medium);
	KGameDifficulty::addStandardLevel(KGameDifficulty::Hard);
	KGameDifficulty::addCustomLevel(Diabolical,
		i18nc("A level of difficulty in Sudoku puzzles", "Diabolical"));
	KGameDifficulty::addCustomLevel(Unlimited,
		i18nc("A level of difficulty in Sudoku puzzles", "Unlimited"));
	KGameDifficulty::setRestartOnChange(KGameDifficulty::NoRestartOnChange);

	// Set default value of difficulty.
	if (difficulty < nStandardLevels) {
	    KGameDifficulty::setLevel (standardLevels[difficulty]);
	}
	else {
	    KGameDifficulty::setLevelCustom (difficulty);
	}
	KGameDifficulty::setEnabled (true);

	// Set up a combo box for symmetry of puzzle layout.
	statusBar()->addPermanentWidget (new QLabel (i18n("Symmetry")));
	QComboBox * symmetryBox = new QComboBox (this);
	QObject::connect(symmetryBox, SIGNAL(activated(int)),
		    		this, SLOT(symmetryChanged(int)));
	symmetryBox->setToolTip(i18nc(
		"Symmetry of layout of clues when puzzle starts", "Symmetry"));
	symmetryBox->setWhatsThis(i18n(
		"The symmetry of layout of the clues when the puzzle starts"));
	statusBar()->addPermanentWidget (symmetryBox);
	symmetryBox->addItem(i18nc("Symmetry of layout of clues", "Diagonal"));
	symmetryBox->addItem(i18nc("Symmetry of layout of clues", "Central"));
	symmetryBox->addItem(i18nc("Symmetry of layout of clues", "Left-Right"));
	symmetryBox->addItem(i18nc("Symmetry of layout of clues", "Spiral"));
	symmetryBox->addItem(i18nc("Symmetry of layout of clues", "Four-Way"));
	symmetryBox->addItem(i18nc("Symmetry of layout of clues", "Random Choice"));
	symmetryBox->addItem(i18n("No Symmetry"));
	symmetryBox->setCurrentIndex (symmetry);
}

void KSudoku::adaptActions2View() {
	Game game = currentGame();

	m_gameSave->setEnabled(game.isValid());
	m_gameSaveAs->setEnabled(game.isValid());
	if(game.isValid()) {
		action("move_undo")->setEnabled(game.canUndo());
		action("move_redo")->setEnabled(game.canRedo());

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

	// only show question when the current game hasn't been finished until now
	if(!m_gameUI->game().wasFinished()) {
		if(KMessageBox::questionYesNo(this,
	                              i18n("Do you really want to end this game in order to start a new one?"),
	                              i18nc("window title", "Restart Game"),
	                              KGuiItem(i18nc("button label", "Restart Game")),
	                              KStandardGuiItem::cancel() ) != KMessageBox::Yes)
			return;
	}

	showWelcomeScreen();
}

void KSudoku::gameOpen()
{
	// this slot is called whenever the Game->Open menu is selected,
	// the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
	// button is clicked
	// standard filedialog
	KUrl Url = KFileDialog::getOpenUrl(KUrl("kfiledialog:///ksudoku"), QString(), this, i18n("Open Location"));

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

	if(game.getUrl().isEmpty()) game.setUrl(KFileDialog::getSaveUrl(KUrl("kfiledialog:///ksudoku")));
 	if (!game.getUrl().isEmpty() && game.getUrl().isValid())
		ksudoku::Serializer::store(game, game.getUrl(), this);
}

void KSudoku::gameSaveAs()
{
    // this slot is called whenever the Game->Save As menu is selected,
	Game game = currentGame();
	if(!game.isValid()) return;

	game.setUrl(KFileDialog::getSaveUrl(KUrl("kfiledialog:///ksudoku")));
    if (!game.getUrl().isEmpty() && game.getUrl().isValid())
    	gameSave();
}


void KSudoku::gamePrint()
{
    // This slot is called whenever the Game->Print action is selected.
    Game game = currentGame();
    if (! game.isValid()) {
        KMessageBox::information (this,
            i18n("There seems to be no puzzle to print."));
        return;
    }
    sendToPrinter (game.puzzle());
}

void KSudoku::sendToPrinter (const Puzzle * puzzle)
{
    const SKGraph * graph = puzzle->graph();
    if (graph->sizeZ() > 1) {
        KMessageBox::information (this,
            i18n("Sorry, cannot print three-dimensional puzzles."));
        return;
    }
    const bool printMulti = Settings::printMulti();
    const int  across  = 2;
    const int  down    = 2;
    const QString labels = (graph->base() <= 3) ? "123456789" :
                                                  "ABCDEFGHIJKLMNOPQRSTUVWXY";
    enum Edge {Left = 0, Right, Above, Below, Detached};
    const int All = (1 << Left) + (1 << Right) + (1 << Above) + (1 << Below);

    // The printer and painter objects can persist between print requests, so
    // (if required) we can print several puzzles per page and defer printing
    // until the page is full or KSudoku terminates and the painter ends itself.
    // NOTE: Must create painter before using functions like m_printer->width().
    if (m_printer == 0) {
        m_printer = new QPrinter (QPrinter::HighResolution);
        QPrintDialog * dialog = new QPrintDialog(m_printer, this);
        dialog->setWindowTitle(i18n("Print Sudoku Puzzle"));
        if (dialog->exec() != QDialog::Accepted) {
            delete m_printer;
            m_printer = 0;
            return;
        }
    }
    if (m_p == 0) {
        m_p = new QPainter (m_printer);	// Start a new print job.
    }

    QVector<int> edges (graph->size(), 0);
    int order = graph->order();

    for (int n = 0; n < graph->cliqueCount(); n++) {
        // Find out which groups are blocks of cells, not rows or columns.
        QVector<int> clique = graph->clique (n);
        int x = graph->cellPosX (clique.at (0));
        int y = graph->cellPosY (clique.at (0));
        bool isRow = true;
        bool isCol = true;
        for (int k = 1; k < order; k++) {
            if (graph->cellPosX (clique.at (k)) != x) isRow = false;
            if (graph->cellPosY (clique.at (k)) != y) isCol = false;
        }
        if (isRow || isCol) continue;	// Skip rows and columns.

        // Mark the outside edges of each block.
        for (int k = 0; k < order; k++) {
            int cell = clique.at (k);
            int x = graph->cellPosX (cell);
            int y = graph->cellPosY (cell);
            int nb[4] = {-1, -1, -1, -1};
            int lim = graph->sizeX() - 1;

            // Start with all edges: remove them as neighbours are found.
            int edge = All;
            nb[Left]  = (x > 0)   ? graph->cellIndex (x-1, y) : -1;
            nb[Right] = (x < lim) ? graph->cellIndex (x+1, y) : -1;
            nb[Above] = (y > 0)   ? graph->cellIndex (x, y-1) : -1;
            nb[Below] = (y < lim) ? graph->cellIndex (x, y+1) : -1;
            for (int neighbour = 0; neighbour < 4; neighbour++) {
                if ((nb[neighbour] < 0) || (puzzle->value(nb[neighbour]) < 0)) {
                    continue;		// No neighbour on this side.
                }
                for (int cl = 0; cl < order; cl++) {
                    if (clique.at (cl) == nb[neighbour]) {
                        edge = edge - (1 << neighbour);
                    }
                }
            }
            edge = (edge == All) ? (1 << Detached) : edge;
            edges [cell] |= edge;	// Merge the edges found for this cell.
        }
    }

    // Calculate the printed dimensions of the puzzle.
    m_printer->setFullPage (false);		// Allow minimal margins.
    int pixels  = qMin (m_printer->width(), m_printer->height());
    int size    = pixels - (pixels / 20);	// Allow about 2.5% each side.
    int divs    = (graph->sizeX() > 20) ? graph->sizeX() : 20;
    int sCell   = size / divs;			// Size of each cell.
    size        = graph->sizeX() * sCell;	// Size of the whole puzzle.

    // Check if we require more than one puzzle per page and if they would fit.
    bool manyUp = printMulti && (pixels > (across * size));
    int margin1 = manyUp ? (pixels - across * size) / (across + 1)	// > 1.
                         : (pixels - size) / 2;				// = 1.
    pixels      = qMax (m_printer->width(), m_printer->height());
    int margin2 = manyUp ? (pixels - down * size) / (down + 1)		// > 1.
                         : (pixels - size) / 2;				// = 1.

    // Check for landscape vs. portrait mode and set the margins accordingly.
    int topX    = (m_printer->width() < m_printer->height())? margin1 : margin2;
    int topY    = (m_printer->width() < m_printer->height())? margin2 : margin1;

    if ((m_quadrant > 0) && (! manyUp)) {
        bool test = m_printer->newPage();	// Page has previous output.
        m_quadrant = 0;
    }
    topX = manyUp ? topX + (m_quadrant % across) * (topX + size) : topX;
    topY = manyUp ? topY + (m_quadrant / across) * (topY + size) : topY;
    m_quadrant = manyUp ? (m_quadrant + 1) : (across * down);

    // Set up parameters for the heavy and light line-drawing.
    int thin    = sCell / 40;	// Allow 2.5%.
    int thick   = (thin > 0) ? 3 * thin : 3;
    int nLines  = graph->order() + 1;

    QPen light (QColor("#888888"));
    QPen heavy (QColor(QString("black")));
    light.setWidth (thin);
    heavy.setWidth (thick);
    heavy.setCapStyle (Qt::RoundCap);

    // Set font size 60% height of cell. Do not draw gray lines on top of black.
    QFont    f = m_p->font();
    f.setPixelSize ((sCell * 6) / 10);
    m_p->setFont (f);
    m_p->setCompositionMode (QPainter::CompositionMode_Darken);

    // Draw each cell in the puzzle.
    for (int n = 0; n < graph->size(); n++) {
        if (puzzle->value (n) < 0) {
            continue;				// Do not draw unused cells.
        }
        int x = topX + sCell * graph->cellPosX (n);
        int y = topY + sCell * graph->cellPosY (n);
        QRect rect (x, y, sCell, sCell);
        int edge = edges.at (n);
        if (edge & (1 << Detached)) {		// Shade a cell, as in XSudoku.
            m_p->fillRect (rect, QColor ("#DDDDDD"));
        }
        m_p->setPen (light);			// First draw every cell light.
        m_p->drawRect (rect);
        m_p->setPen (heavy);			// Draw block boundaries heavy.
        if (edge & (1<<Left))  m_p->drawLine (x, y, x, y + sCell);
        if (edge & (1<<Right)) m_p->drawLine (x+sCell, y, x+sCell, y+sCell);
        if (edge & (1<<Above)) m_p->drawLine (x, y, x+sCell, y);
        if (edge & (1<<Below)) m_p->drawLine (x, y+sCell, x+sCell, y+sCell);

        // Draw original puzzle values heavy: filled-in/solution values light.
        int v = currentGame().value (n) - 1;
        if (v >= 0) {				// Skip empty cells.
            m_p->setPen ((puzzle->value (n) > 0) ? heavy : light);
            m_p->drawText (rect, Qt::AlignCenter, labels.mid (v, 1));
        }
    }
    if ((! manyUp) || (m_quadrant >= (across * down))) {
        endPrint();				// Print immediately.
    }
    else {
        KMessageBox::information (this,
            i18n ("The KSudoku setting for printing several puzzles per page "
                  "is currently selected.\n\n"
                  "Your puzzle will be printed when no more will fit on the "
                  "page or when KSudoku terminates."));
    }
}

void KSudoku::endPrint()
{
    if (m_p != 0) {
        // The current print output goes to the printer when the painter ends.
        m_p->end();
        delete m_p;
        m_p = 0;
        m_quadrant = 0;
        KMessageBox::information (this,
            i18n ("KSudoku has sent output to your printer."));
    }
}

bool KSudoku::queryClose()
{
    endPrint();
    return true;
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
	connect(dialog, SIGNAL(settingsChanged(QString)), SLOT(updateSettings()));
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

void KSudoku::difficultyChanged (KGameDifficulty::standardLevel difficulty)
{
    qDebug() << "Set difficulty =" << difficulty;
    int newDifficulty = VeryEasy;
    switch (difficulty) {
    case KGameDifficulty::VeryEasy:
	newDifficulty = VeryEasy;
	break;
    case KGameDifficulty::Easy:
	newDifficulty = Easy;
	break;
    case KGameDifficulty::Medium:
	newDifficulty = Medium;
	break;
    case KGameDifficulty::Hard:
	newDifficulty = Hard;
	break;
    default:
	return;
    }
    qDebug() << "Set new difficulty =" << newDifficulty;
    m_welcomeScreen->setDifficulty(newDifficulty);
    return;
}

void KSudoku::difficultyChanged (int difficulty)
{
    qDebug() << "Set custom difficulty =" << difficulty;
    m_welcomeScreen->setDifficulty(difficulty);
    if (difficulty == Unlimited) {
	KMessageBox::information (this,
		i18n("Warning: The Unlimited difficulty level has no limit on "
		     "how many guesses or branch points are required to solve "
		     "the puzzle and there is no lower limit on how soon "
		     "guessing becomes necessary."),
		i18n("Warning"), "WarningReUnlimited");
    }
}

void KSudoku::symmetryChanged (int symmetry)
{
    qDebug() << "Set symmetry =" << symmetry;
    m_welcomeScreen->setSymmetry(symmetry);
}

// void KSudoku::changeStatusbar(const QString& text)
// {
//     // display the text on the statusbar
//     statusBar()->showMessage(text);
// }

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
