/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2008 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
 *   Copyright 2012,2015 Ian Wadham <iandw.au@gmail.com>                   *
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

#include "ksudoku_logging.h"
#include "globals.h"
#include "ksudoku.h"

#include <QAction>
#include <QComboBox>
#include <QDir>
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>
#include <QLabel>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QPrinter>
#include <QPrintDialog>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTemporaryFile>
#include <QUrl>

#include <KActionCollection>
#include <KConfigDialog>
#include <KIO/Job>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRun>
#include <KSharedConfig>
#include <KStandardAction>
#include <KStandardGameAction>
#include <KTar>

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgamethemeselector.h>

#include "ksview.h"
#include "gameactions.h"
#include "renderer.h"

#include "puzzle.h" // TODO
#include "skgraph.h"
#include "serializer.h"

#include "puzzleprinter.h"

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
	m_puzzlePrinter(0)
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
	connect(m_welcomeScreen, &ksudoku::WelcomeScreen::newGameStarted, this, &KSudoku::startGame);

	setupStatusBar(m_welcomeScreen->difficulty(),
		       m_welcomeScreen->symmetry());

	showWelcomeScreen();

	updateShapesList();

// 	QTimer *timer = new QTimer( this );
// 	connect( timer, SIGNAL(timeout()), this, SLOT(updateStatusBar()) );
// 	updateStatusBar();
// 	timer->start( 1000); //TODO PORT, false ); // 2 seconds single-shot timer
}

KSudoku::~KSudoku()
{
	delete m_puzzlePrinter;
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

	QStringList gamevariantdirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "ksudoku", QStandardPaths::LocateDirectory);

	QStringList filepaths;
	Q_FOREACH (const QString& gamevariantdir, gamevariantdirs) {
		const QStringList fileNames = QDir(gamevariantdir).entryList(QStringList() << QStringLiteral("*.desktop"));
		Q_FOREACH (const QString &file, fileNames) {
			if (!filepaths.contains(gamevariantdir + '/' + file)) {
				filepaths.append(gamevariantdir + '/' + file);
			}
		}
	}

	QString variantName;
	QString variantDescr;
	QString variantDataPath;
	QString variantIcon;

	foreach(const QString &filepath, filepaths) {
		const QFileInfo configFileInfo(filepath);
		const QDir variantDir = configFileInfo.dir();
		KConfig variantConfig(filepath, KConfig::SimpleConfig);
		KConfigGroup group = variantConfig.group ("KSudokuVariant");

		variantName = group.readEntry("Name", i18n("Missing Variant Name")); // Translated.
		variantDescr = group.readEntry("Description", ""); // Translated.
		variantIcon = group.readEntry("Icon", "ksudoku-ksudoku_9x9");
		const QString variantDataFile = group.readEntry("FileName", "");
		if(variantDataFile == "") continue;

		variantDataPath = variantDir.filePath(variantDataFile);

		variant = new CustomGame(variantName, QUrl::fromLocalFile(variantDataPath), m_gameVariants);
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

	connect(view, &KsView::valueSelected, m_valueListWidget, &ksudoku::ValueListWidget::selectValue);
	connect(m_valueListWidget, &ksudoku::ValueListWidget::valueSelected, view, &KsView::selectValue);
// 	connect(view, SIGNAL(valueSelected(int)), SLOT(updateStatusBar()));

	QWidget* widget = view->widget();
	m_gameUI = view;
	Game g = currentGame();
	g.setMessageParent(view->widget());

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

	SudokuType t = game.puzzle()->graph()->specificType();
	bool playing = game.puzzle()->hasSolution();
	if (playing && (t == Mathdoku)) {
	    KMessageBox::information (this,
		i18n("Mathdoku puzzles can have any size from 3x3 up to 9x9. "
		     "The solution is a grid in which every row and every "
		     "column contains the available digits (1-3 up to 1-9) "
		     "exactly once. The grid is covered with irregularly "
		     "shaped cages.\n"
		     "\n"
		     "Cages of size 1 are starting-values or clues, but there "
		     "are not many of them. Cages of larger size have a target "
		     "value and an arithmetic operator (+-x/). The digits in "
		     "the cage must combine together, using the operator, to "
		     "reach the target value, e.g. '12x' means that the digits "
		     "must multiply together to make 12. A digit can occur "
		     "more than once in a cage, provided it occurs in "
		     "different rows and columns.\n"
		     "\n"
		     "In general, larger Mathdokus are more difficult and so "
		     "are larger cages. You can select the puzzle size in "
		     "KSudoku's Settings dialog and the maximum cage-size by "
		     "using KSudoku's Difficulty button."),
		i18n("Playing Mathdoku"), QString("PlayingMathdoku"));
	}
	else if (playing && (t == KillerSudoku)) {
	    KMessageBox::information (this,
		i18n("Killer Sudoku puzzles can have sizes 4x4 or 9x9, with "
		     "either four 2x2 blocks or nine 3x3 blocks. The solution "
		     "must follow Classic Sudoku rules. The difference is that "
		     "there are few starting-values or clues (if any). Instead "
		     "the grid is covered with irregularly shaped cages.\n"
		     "\n"
		     "Cages of size 1 are starting-values or clues. Cages of "
		     "larger size have a target value and the digits in them "
		     "must add up to that value. In Killer Sudoku, a cage "
		     "cannot contain any digit more than once.\n"
		     "\n"
		     "In general, larger cages are more difficult. You can "
		     "select the maximum cage-size by using KSudoku's "
		     "Difficulty button."),
		i18n("Playing Killer Sudoku"), QString("PlayingKillerSudoku"));
	}
	else if ((t == Mathdoku) || (t == KillerSudoku)) {
	    KMessageBox::information (this,
		i18n("Mathdoku and Killer Sudoku puzzles have to be keyed in "
		     "by working on one cage at a time. To start a cage, left "
		     "click on any unused cell or enter a number in the cell "
		     "that is under the cursor or enter + - / or x there. A "
		     "small cage-label will appear in that cell. To extend the "
		     "cage in any direction, left-click on a neigbouring cell "
		     "or move the cursor there and type a Space.\n"
		     "\n"
		     "The number you type is the cage's value and it can have "
		     "one or more digits, including zero. A cell of size 1 has "
		     "to have a 1-digit number, as in a normal Sudoku puzzle. "
		     "It becomes a starting-value or clue for the player.\n"
		     "\n"
		     "The + - / or x is the operator (Add, Subtract, Divide or "
		     "Multiply). You must have one in cages of size 2 or more. "
		     "In Killer Sudoku, the operator is provided automatically "
		     "because it is always + or none.\n"
		     "\n"
		     "You can enter digits, operators and cells in any order. "
		     "To complete the cage and start another cage, always "
		     "press Return. If you make a mistake, the only thing to "
		     "do is delete a whole cage and re-enter it. Use right "
		     "click in the current cage or any earlier cage, if you "
		     "wish to delete it. Alternatively, use the cursor and the "
		     "Delete or Backspace key.\n"
		     "\n"
		     "When the grid is filled with cages, hit the Check "
		     "button, to solve the puzzle and make sure there is only "
		     "one solution. If the check fails, you have probably made "
		     "an error somewhere in one of the cages."),
		i18n("Data-entry for Puzzles with Cages"),
		QString("CageDataEntry"));
	}
}

void KSudoku::endCurrentGame() {
	m_valueListWidget->hide();
	
	delete m_gameUI;
	m_gameUI = 0;
	
	adaptActions2View();

}

void KSudoku::loadGame(const QUrl& Url) {
	QString errorMsg;
	const Game game = ksudoku::Serializer::load(Url, this, errorMsg);
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
	KRun::runUrl (QUrl("http://ksudoku.sourceforge.net/"), "text/html", this, KRun::RunFlags());
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
		KMessageBox::information (this,
		    i18n("Sorry, no solutions have been found. Please check "
			 "that you have entered in the puzzle completely and "
			 "correctly."), i18n("Check Puzzle"));
		delete puzzle;
		return;
	} else if(state == 1) {
		KMessageBox::information (this,
		    i18n("The Puzzle you entered has a unique solution and "
			 "is ready to be played."), i18n("Check Puzzle"));
	} else {
		KMessageBox::information (this,
		    i18n("The Puzzle you entered has multiple solutions. "
			 "Please check that you have entered in the puzzle "
			 "completely and correctly."), i18n("Check Puzzle"));
	}

	if(KMessageBox::questionYesNo(this, i18n("Do you wish to play the puzzle now?"), i18n("Play Puzzle"), KGuiItem(i18n("Play")), KStandardGuiItem::cancel() ) == KMessageBox::Yes)
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

	QKeySequence shortcut;

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
	// Settings: enable messages that the user marked "Do not show again".
	QAction* enableMessagesAct = new QAction(i18n("Enable all messages"),0);
	actionCollection()->addAction("enable_messages", enableMessagesAct);
	connect(enableMessagesAct, SIGNAL(triggered()), SLOT(enableMessages()));

	//History
	KStandardGameAction::undo(this, SLOT(undo()), actionCollection());
	KStandardGameAction::redo(this, SLOT(redo()), actionCollection());

	QAction * a = KStandardGameAction::hint(this, SLOT(giveHint()), actionCollection());
	// The default value (H) conflicts with the keys assigned
	// to add letter/numbers to the board.
	actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::Key_F2));

	KStandardGameAction::solve(this, SLOT(autoSolve()), actionCollection());

	a = new QAction(this);
	actionCollection()->addAction( QLatin1String( "move_dub_puzzle" ), a);
	a->setText(i18n("Check"));
	a->setIcon(QIcon::fromTheme( QLatin1String( "games-endturn" )));
	connect(a, &QAction::triggered, this, &KSudoku::dubPuzzle);
	addAction(a);

	//WEB
	a = new QAction(this);
	actionCollection()->addAction( QLatin1String( "home_page" ), a);
	a->setText(i18n("Home Page"));
	a->setIcon(QIcon::fromTheme( QLatin1String( "internet-web-browser" )));
	connect(a, &QAction::triggered, this, &KSudoku::homepage);
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
	QObject::connect(symmetryBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &KSudoku::symmetryChanged);
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

void KSudoku::dragEnterEvent(QDragEnterEvent * event)
{
    // accept uri drops only
    if(event->mimeData()->hasUrls())
        event->accept();
}

void KSudoku::dropEvent(QDropEvent *event)
{
    const QMimeData * data = event->mimeData();
    if(data->hasUrls())
    {
        QList<QUrl> Urls = data->urls();

        if ( !Urls.isEmpty() )
        {
            // okay, we have a URI.. process it
            const QUrl &Url = Urls.first();

            QString errorMsg;
            const Game game = ksudoku::Serializer::load(Url, this, errorMsg);
            if(game.isValid())
                startGame(game);
            else
                KMessageBox::error(this, errorMsg, i18n("Could not load game."));
        }
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
	const QUrl Url = QFileDialog::getOpenFileUrl(this, i18n("Open Location"), QUrl::fromLocalFile(QDir::homePath()), QString());

	if (!Url.isEmpty() && Url.isValid())
	{
		QString errorMsg;
		Game game = ksudoku::Serializer::load(Url, this, errorMsg);
		if(!game.isValid()) {
			KMessageBox::error(this, errorMsg, i18n("Could not load game."));
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

	if(game.getUrl().isEmpty()) game.setUrl(QFileDialog::getSaveFileUrl());
	if (!game.getUrl().isEmpty() && game.getUrl().isValid())
	{
		QString errorMsg;
		if (!ksudoku::Serializer::store(game, game.getUrl(), this, errorMsg))
			KMessageBox::error(this, errorMsg, i18n("Error Writing File"));
	}
}

void KSudoku::gameSaveAs()
{
    // this slot is called whenever the Game->Save As menu is selected,
	Game game = currentGame();
	if(!game.isValid()) return;

	game.setUrl(QFileDialog::getSaveFileUrl());
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
    if (! m_puzzlePrinter) {
	m_puzzlePrinter = new PuzzlePrinter (this);
    }
    m_puzzlePrinter->print (game);
}

bool KSudoku::queryClose()
{
    if (m_puzzlePrinter) {
	m_puzzlePrinter->endPrint();
    }
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

    //QT5 dialog->setHelp(QString(),"ksudoku");
	connect(dialog, &KConfigDialog::settingsChanged, this, &KSudoku::updateSettings);
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
    qCDebug(KSudokuLog) << "Set difficulty =" << difficulty;
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
    qCDebug(KSudokuLog) << "Set new difficulty =" << newDifficulty;
    m_welcomeScreen->setDifficulty(newDifficulty);
    return;
}

void KSudoku::difficultyChanged (int difficulty)
{
    qCDebug(KSudokuLog) << "Set custom difficulty =" << difficulty;
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
    qCDebug(KSudokuLog) << "Set symmetry =" << symmetry;
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

void KSudoku::enableMessages()
{
	// Enable all messages that the user has marked "Do not show again".
	int result = KMessageBox::questionYesNo(this,
					i18n("Enable all messages"));
	if (result == KMessageBox::Yes) {
		KMessageBox::enableAllMessages();
		KSharedConfig::openConfig()->sync();	// Save the changes to disk.
	}
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
	const QString destDir = QStandardPaths::writableLocation( QStandardPaths::GenericDataLocation ) + QStringLiteral("/ksudoku/");
	QDir().mkpath(destDir);

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




