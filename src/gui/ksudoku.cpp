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

#include "ksudoku.h"
#include "ksudoku_logging.h"
#include "globals.h"

#include <QAction>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>
#include <QMimeDatabase>
#include <QLabel>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QStandardPaths>
#include <QStatusBar>
#include <QUrl>


#include <KActionCollection>
#include <KConfigDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KStandardAction>
#include <KTar>

#include <KGameStandardAction>
#include <KGameThemeSelector>
#include <KGameDifficulty>

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

static QString openSaveFilterString() {
	static QString filterString;
	if (filterString.isEmpty()) {
		QStringList mimeFilters;
		QMimeDatabase db;
		mimeFilters += db.mimeTypeForName(QStringLiteral("application/xml")).filterString();
		mimeFilters += db.mimeTypeForName(QStringLiteral("text/plain")).filterString();
		mimeFilters += i18n("All Files (*)");
		filterString = mimeFilters.join(QStringLiteral(";;"));
	}
	return filterString;
}

void KSudoku::onCompleted(bool isCorrect, QTime required, bool withHelp) {
	if(!isCorrect) {
		KMessageBox::information(this, i18n("Sorry, your solution contains mistakes.\n\nEnable \"Show errors\" in the settings to highlight them."));
		return;
	}

	m_timer->stop();
	setTimerDisplay();
	m_pauseButton->setEnabled(false);

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
	else if(m_wasPaused)
		if (mins == 0)
			msg = i18np("Congratulations! You made it in 1 second. With at least 1 pause.", "Congratulations! You made it in %1 seconds. With at least 1 pause.", secs);
		else if (secs == 0)
			msg = i18np("Congratulations! You made it in 1 minute. With at least 1 pause.", "Congratulations! You made it in %1 minutes. With at least 1 pause.", mins);
		else
			msg = i18nc("The two parameters are strings like '2 minutes' or '1 second'.", "Congratulations! You made it in %1 and %2. With at least 1 pause.", i18np("1 minute", "%1 minutes", mins), i18np("1 second", "%1 seconds", secs));
	else
		if (mins == 0)
			msg = i18np("Congratulations! You made it in 1 second.", "Congratulations! You made it in %1 seconds.", secs);
		else if (secs == 0)
			msg = i18np("Congratulations! You made it in 1 minute.", "Congratulations! You made it in %1 minutes.", mins);
		else
			msg = i18nc("The two parameters are strings like '2 minutes' or '1 second'.", "Congratulations! You made it in %1 and %2.", i18np("1 minute", "%1 minutes", mins), i18np("1 second", "%1 seconds", secs));

    onModified(true); // make sure buttons have the correct enabled state

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
    m_puzzlePrinter(nullptr)
{
	setObjectName( QStringLiteral("ksudoku" ));

	// Set game has been paused to false
	m_wasPaused = false;

	m_gameWidget = nullptr;
    m_gameUI = nullptr;

    m_gameActions = nullptr;
	m_timeDisplay = new QLabel;

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

	connect(Renderer::instance()->themeProvider(), &KGameThemeProvider::currentThemeChanged, this, &KSudoku::updateSettings);

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
    GameVariant* variant = nullptr;

	variant = new SudokuGame(this, i18n("Sudoku Standard (9x9)"), 9, m_gameVariants);
	variant->setDescription(i18n("The classic and fashionable game"));
	variant->setIcon(QStringLiteral("ksudoku-ksudoku_9x9"));
#ifdef OPENGL_SUPPORT
	variant = new RoxdokuGame(this, i18n("Roxdoku 9 (3x3x3)"), 9, m_gameVariants);
	variant->setDescription(i18n("The Rox 3D Sudoku"));
	variant->setIcon(QStringLiteral("ksudoku-roxdoku_3x3x3"));
#endif

	const QStringList gamevariantdirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("ksudoku"), QStandardPaths::LocateDirectory);

	QFileInfoList filepaths;
	for (const QString& gamevariantdir : gamevariantdirs) {
		const auto fileNames = QDir(gamevariantdir).entryInfoList(QStringList() << QStringLiteral("*.desktop"), QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);
		filepaths.append(fileNames);
	}

	QString variantName;
	QString variantDescr;
	QString variantDataPath;
	QString variantIcon;

    for (const QFileInfo &configFileInfo : std::as_const(filepaths)) {
		const QDir variantDir = configFileInfo.dir();
		KConfig variantConfig(configFileInfo.filePath(), KConfig::SimpleConfig);
		KConfigGroup group = variantConfig.group (QStringLiteral("KSudokuVariant"));

		variantName = group.readEntry("Name", i18n("Missing Variant Name")); // Translated.
		variantDescr = group.readEntry("Description", ""); // Translated.
		variantIcon = group.readEntry("Icon", "ksudoku-ksudoku_9x9");
		const QString variantDataFile = group.readEntry("FileName", QString());
		if (variantDataFile.isEmpty()) {
		    continue;
		}

		variantDataPath = variantDir.filePath(variantDataFile);

		variant = new CustomGame(this, variantName, QUrl::fromLocalFile(variantDataPath), m_gameVariants);
		variant->setDescription(variantDescr);
		variant->setIcon(variantIcon);
	}

	// Put variants first and extra sizes last.
	variant = new SudokuGame(this, i18n("Sudoku 16x16"), 16, m_gameVariants);
	variant->setDescription(i18n("Sudoku with 16 symbols"));
	variant->setIcon(QStringLiteral("ksudoku-ksudoku_16x16"));
	variant = new SudokuGame(this, i18n("Sudoku 25x25"), 25, m_gameVariants);
	variant->setDescription(i18n("Sudoku with 25 symbols"));
	variant->setIcon(QStringLiteral("ksudoku-ksudoku_25x25"));
#ifdef OPENGL_SUPPORT
	variant = new RoxdokuGame(this, i18n("Roxdoku 16 (4x4x4)"), 16, m_gameVariants);
	variant->setDescription(i18n("The Rox 3D sudoku with 16 symbols"));
	variant->setIcon(QStringLiteral("ksudoku-roxdoku_4x4x4"));
	variant = new RoxdokuGame(this, i18n("Roxdoku 25 (5x5x5)"), 25, m_gameVariants);
	variant->setDescription(i18n("The Rox 3D sudoku with 25 symbols"));
	variant->setIcon(QStringLiteral("ksudoku-roxdoku_5x5x5"));
#endif
}

void KSudoku::startGame(const Game& game) {
	m_welcomeScreen->hide();
	endCurrentGame();


	auto* view = new KsView(game, m_gameActions, this);

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
	wrapper->setObjectName(QStringLiteral("BG"));
	wrapper->setStyleSheet(QStringLiteral("#BG { background-image: url(:/shapes/icon_pause.png);"
										"background-repeat: no-repeat;"
										"background-position: center; }"
										));

	widget->show();
	widget->setFocus();

	connect(game.interface(), &GameIFace::completed, this, &KSudoku::onCompleted);
	connect(game.interface(), &GameIFace::modified, this, &KSudoku::onModified);

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
		i18n("Playing Mathdoku"), QStringLiteral("PlayingMathdoku"));
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
		i18n("Playing Killer Sudoku"), QStringLiteral("PlayingKillerSudoku"));
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
		QStringLiteral("CageDataEntry"));
	}
}

void KSudoku::gamePause() {

	if(m_pauseButton->isChecked())
	{
		m_wasPaused = true;
		// Disable all other visible game play buttons
		action(QStringLiteral("game_new"))->setEnabled(false);
		action(QStringLiteral("game_restart"))->setEnabled(false);
		action(QStringLiteral("game_print"))->setEnabled(false);
		action(QStringLiteral("move_hint"))->setEnabled(false);
		action(QStringLiteral("move_solve"))->setEnabled(false);

		// Hide/Disable the game widgets
		m_gameUI->widget()->setDisabled(true);
		m_gameUI->widget()->hide();
		m_gameUI->valueListWidget()->hide();

		// Capture the current timer value
		m_gameTime = m_gameUI->game().msecsElapsed();
		m_timer->stop();
	}
	else{
		// Enable all other visible game play buttons
		action(QStringLiteral("game_new"))->setEnabled(true);
		action(QStringLiteral("game_restart"))->setEnabled(true);
		action(QStringLiteral("game_print"))->setEnabled(currentGame().isValid());
		action(QStringLiteral("move_hint"))->setEnabled(!currentGame().allValuesSetAndUsable());
		action(QStringLiteral("move_solve"))->setEnabled(!currentGame().wasFinished());

		// Show/Enable the game widgets
		m_gameUI->widget()->setEnabled(true);
		m_gameUI->widget()->show();
		m_gameUI->valueListWidget()->show();

		// Set the game timer to the previous value
		m_gameUI->game().setTime(m_gameTime);
		m_timer->start();
	}
}

void KSudoku::endCurrentGame() {
	m_valueListWidget->hide();

	delete m_gameUI;
    m_gameUI = nullptr;

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
			 "correctly."),
		    i18nc("@title:window", "Check Puzzle"));
		delete puzzle;
		return;
	} else if(state == 1) {
		KMessageBox::information (this,
		    i18n("The Puzzle you entered has a unique solution and "
			 "is ready to be played."),
		    i18nc("@title:window", "Check Puzzle"));
	} else {
		KMessageBox::information (this,
		    i18n("The Puzzle you entered has multiple solutions. "
			 "Please check that you have entered in the puzzle "
			 "completely and correctly."),
		    i18nc("@title:window", "Check Puzzle"));
	}

	if(KMessageBox::questionTwoActions(this, i18n("Do you wish to play the puzzle now?"),
					   i18nc("@title:window", "Play Puzzle"),
					   KGuiItem(i18nc("@action:button", "Play")),
					   KStandardGuiItem::cancel() ) == KMessageBox::PrimaryAction)
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
	m_gameActions = new ksudoku::GameActions(actionCollection(), this);
	m_gameActions->init();

	QKeySequence shortcut;

	setAcceptDrops(true);

	KGameStandardAction::gameNew(this, &KSudoku::gameNew, actionCollection());
	KGameStandardAction::restart(this, &KSudoku::gameRestart, actionCollection());
	KGameStandardAction::load(this, &KSudoku::gameOpen, actionCollection());
	m_pauseButton = KGameStandardAction::pause(this, &KSudoku::gamePause, actionCollection());
	KActionCollection::setDefaultShortcut(m_pauseButton, QKeySequence(Qt::Key_F8));
	m_gameSave = KGameStandardAction::save(this, &KSudoku::gameSave, actionCollection());
	m_gameSaveAs = KGameStandardAction::saveAs(this, &KSudoku::gameSaveAs, actionCollection());
	KGameStandardAction::print(this, &KSudoku::gamePrint, actionCollection());
	KGameStandardAction::quit(this, &KSudoku::close, actionCollection());
	// TODO Export is disabled due to missing port to KDE4.
// 	createAction("game_export", SLOT(gameExport()), i18n("Export"));

	KStandardAction::preferences(this, &KSudoku::optionsPreferences, actionCollection());
	// Settings: enable messages that the user marked "Do not show again".
	auto* enableMessagesAct = new QAction(i18nc("@action", "Enable All Messages"),this);
	actionCollection()->addAction(QStringLiteral("enable_messages"), enableMessagesAct);
	connect(enableMessagesAct, &QAction::triggered, this, &KSudoku::enableMessages);

	//History
	KGameStandardAction::undo(this, &KSudoku::undo, actionCollection());
	KGameStandardAction::redo(this, &KSudoku::redo, actionCollection());

	QAction * a = KGameStandardAction::hint(this, &KSudoku::giveHint, actionCollection());
	// The default value (H) conflicts with the keys assigned
	// to add letter/numbers to the board.
	KActionCollection::setDefaultShortcut(a, QKeySequence(Qt::Key_F2));

	KGameStandardAction::solve(this, &KSudoku::autoSolve, actionCollection());

	a = new QAction(this);
	actionCollection()->addAction( QStringLiteral( "move_dub_puzzle" ), a);
	a->setText(i18nc("@action", "Check"));
	a->setIcon(QIcon::fromTheme( QStringLiteral( "games-endturn" )));
	connect(a, &QAction::triggered, this, &KSudoku::dubPuzzle);
	addAction(a);
}

void KSudoku::setupStatusBar (int difficulty, int symmetry)
{
	// Use the standard combo box for difficulty, from KDE Games library.
	KGameDifficulty::global()->addStandardLevelRange(KGameDifficultyLevel::VeryEasy, KGameDifficultyLevel::Hard);
	// and add our custom ones
	enum CustomHardness {
	    DiabolicalHardness = KGameDifficultyLevel::Hard + 1,
	    UnlimitedHardness
	};
	KGameDifficulty::global()->addLevel(new KGameDifficultyLevel(DiabolicalHardness, "Diabolical",
		i18nc("A level of difficulty in Sudoku puzzles", "Diabolical")));
	KGameDifficulty::global()->addLevel(new KGameDifficultyLevel(UnlimitedHardness, "Unlimited",
		i18nc("A level of difficulty in Sudoku puzzles", "Unlimited")));

	// Set default value of difficulty
	const KGameDifficultyLevel * level = KGameDifficulty::global()->levels().value(difficulty);
	if (level) {
	    KGameDifficulty::global()->select(level);
	}

	connect(KGameDifficulty::global(), &KGameDifficulty::currentLevelChanged,
		this, &KSudoku::handleCurrentDifficultyLevelChanged);

	statusBar()->addPermanentWidget (new QLabel (i18nc("@option drop down box", "Difficulty:")));
	KGameDifficultyGUI::init(this);

	// Set up a combo box for symmetry of puzzle layout.
	statusBar()->addPermanentWidget (new QLabel (i18nc("@option drop down box", "Symmetry:")));
	auto * symmetryBox = new QComboBox (this);
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

	// Setup the timer display in the status bar
	QFont font(QStringLiteral("Monospace"));
	font.setFixedPitch(true);
	m_timeDisplay->setFrameStyle(QFrame::NoFrame);
	m_timeDisplay->setFont(font);
	m_timeDisplay->setText(QStringLiteral("00:00:00"));
	statusBar()->addWidget (new QLabel (i18nc("@action", "Elapsed Time:")));
	statusBar()->addWidget (m_timeDisplay);
	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, &KSudoku::setTimerDisplay);
	m_timer->setInterval(500);
	m_timer->start();
}

void KSudoku::adaptActions2View() {
	Game game = currentGame();

	m_gameSave->setEnabled(game.isValid());
	m_gameSaveAs->setEnabled(game.isValid());
	action(QStringLiteral("game_new"))->setEnabled(game.isValid());
	action(QStringLiteral("game_restart"))->setEnabled(game.isValid());
	action(QStringLiteral("game_pause"))->setEnabled(game.isValid());
	action(QStringLiteral("game_print"))->setEnabled(game.isValid());
	if(game.isValid()) {
		bool isEnterPuzzleMode = !game.puzzle()->hasSolution();
		action(QStringLiteral("move_hint"))->setVisible(!isEnterPuzzleMode);
		action(QStringLiteral("move_solve"))->setVisible(!isEnterPuzzleMode);
		action(QStringLiteral("move_dub_puzzle"))->setVisible(isEnterPuzzleMode);

		action(QStringLiteral("move_undo"))->setEnabled(game.canUndo());
		action(QStringLiteral("move_redo"))->setEnabled(game.canRedo());

		action(QStringLiteral("move_hint"))      ->setEnabled(   game.puzzle()->hasSolution());
		action(QStringLiteral("move_solve"))     ->setEnabled(   game.puzzle()->hasSolution());
		action(QStringLiteral("move_dub_puzzle"))->setEnabled( ! game.puzzle()->hasSolution());
	} else {
		action(QStringLiteral("move_undo"))->setEnabled(false);
		action(QStringLiteral("move_redo"))->setEnabled(false);

		action(QStringLiteral("move_hint"))->setVisible(false);
		action(QStringLiteral("move_solve"))->setVisible(false);
		action(QStringLiteral("move_dub_puzzle"))->setVisible(false);
	}
}

void KSudoku::onModified(bool /*isModified*/) {
	Game game = currentGame();
	if(game.isValid()) {
		action(QStringLiteral("move_undo"))->setEnabled(game.canUndo());
		action(QStringLiteral("move_redo"))->setEnabled(game.canRedo());
		action(QStringLiteral("move_hint"))->setEnabled(!game.allValuesSetAndUsable());
		action(QStringLiteral("move_solve"))->setEnabled(!game.wasFinished());
	}
}

void KSudoku::undo() {
	Game game = currentGame();
	if(!game.isValid()) return;

	game.interface()->undo();

	if(!game.canUndo()) {
		action(QStringLiteral("move_undo"))->setEnabled(false);
	}
}

void KSudoku::redo() {
	Game game = currentGame();
	if(!game.isValid()) return;

	game.interface()->redo();

	if(!game.canRedo()) {
		action(QStringLiteral("move_redo"))->setEnabled(false);
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
                KMessageBox::error(this, errorMsg, i18nc("@title:window", "Error Loading Game"));
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
		if(KMessageBox::questionTwoActions(this,
	                              i18n("Do you really want to end this game in order to start a new one?"),
	                              i18nc("window title", "New Game"),
	                              KGuiItem(i18nc("@action:button", "New Game"), QStringLiteral("document-new")),
	                              KStandardGuiItem::cancel() ) != KMessageBox::PrimaryAction)
			return;
	}

	showWelcomeScreen();
}

void KSudoku::gameRestart()
{
	if (!currentView()) return;

	auto game = currentGame();
	// only show question when the current game hasn't been finished until now
	if (!game.wasFinished()) {
		if (KMessageBox::questionTwoActions(this,
                                i18n("Do you really want to restart this game?"),
                                i18nc("window title", "Restart Game"),
                                KGuiItem(i18nc("@action:button", "Restart Game"), QStringLiteral("view-refresh")),
                                KStandardGuiItem::cancel() ) != KMessageBox::PrimaryAction) {
			return;
		}
	}

	m_wasPaused = false;
	action(QStringLiteral("game_pause"))->setEnabled(game.isValid());
	game.restart();
	game.setTime(0);
	m_timer->start();
}

void KSudoku::gameOpen()
{
	// this slot is called whenever the Game->Open menu is selected,
	// the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
	// button is clicked
	// standard filedialog
	const QString docsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
	const QUrl Url = QFileDialog::getOpenFileUrl(this, QString(), QUrl::fromLocalFile(docsDir), openSaveFilterString());

	if (!Url.isEmpty() && Url.isValid())
	{
		QString errorMsg;
		Game game = ksudoku::Serializer::load(Url, this, errorMsg);
		if(!game.isValid()) {
			KMessageBox::error(this, errorMsg, i18nc("@title:window", "Error Loading Game"));
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
	
	if(game.getUrl().isEmpty()) {

		gameSaveAs();
	}
	else if (!game.getUrl().isEmpty() && game.getUrl().isValid())
	{
		QString errorMsg;
		if (!ksudoku::Serializer::store(game, game.getUrl(), this, errorMsg))
			KMessageBox::error(this, errorMsg, i18nc("@title:window", "Error Writing File"));
	}
}

void KSudoku::gameSaveAs()
{
    // this slot is called whenever the Game->Save As menu is selected,
	Game game = currentGame();
	if(!game.isValid()) 
		return;
	const QDir docsDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
	const QString fName = i18n("Game_") +
		 QDateTime::currentDateTime().toString(QStringLiteral("yyyy_MM_dd_hhmmss")) +
		QStringLiteral(".ksudoku.xml");
	const QUrl fileName = QUrl::fromLocalFile(docsDir.filePath(fName));
	game.setUrl(QFileDialog::getSaveFileUrl(this,i18n("Save Game"),fileName, openSaveFilterString()));
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
	if ( KConfigDialog::showDialog(QStringLiteral("settings")) ) return;

	auto *dialog = new KConfigDialog(this, QStringLiteral("settings"), Settings::self());

	auto* gameConfig = new GameConfig();
	dialog->addPage(gameConfig, i18nc("@title:tab Game Section in Config", "Game"), QStringLiteral("games-config-options"));
	dialog->addPage(new KGameThemeSelector(Renderer::instance()->themeProvider()), i18nc("@title:tab", "Theme"), QStringLiteral("games-config-theme"));

    //QT5 dialog->setHelp(QString(),"ksudoku");
	connect(dialog, &KConfigDialog::settingsChanged, this, &KSudoku::updateSettings);
	dialog->show();
}

void KSudoku::updateSettings() {
	KsView* view = currentView();
	if(view) {
		int order = view->game().order();
		m_valueListWidget->setMaxValue(order);

		view->settingsChanged();
	}

	Q_EMIT settingsChanged();
}

void KSudoku::handleCurrentDifficultyLevelChanged(const KGameDifficultyLevel *level)
{
    const int difficulty = KGameDifficulty::global()->levels().indexOf(level);

    if (difficulty == -1) {
	return;
    }

    qCDebug(KSudokuLog) << "Set new difficulty =" << difficulty;
    m_welcomeScreen->setDifficulty(difficulty);

    if (difficulty == Unlimited) {
	KMessageBox::information (this,
		i18n("Warning: The Unlimited difficulty level has no limit on "
		     "how many guesses or branch points are required to solve "
		     "the puzzle and there is no lower limit on how soon "
		     "guessing becomes necessary.\n\n"
		     "Please also note that the generation of this type of puzzle "
		     "might take much longer than other ones. During this time "
		     "KSudoku will not respond."),
		i18nc("@title:window", "Warning"),
		QStringLiteral("WarningReUnlimited"));
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
	int result = KMessageBox::questionTwoActions(this,
					i18n("This will enable all the dialogs that you had disabled by marking "
						 "the 'Do not show this message again' option.\n\n"
						 "Do you want to continue?"),
					QString(),
					KGuiItem(i18nc("@action:button", "Enable")),
					KStandardGuiItem::cancel());
	if (result == KMessageBox::PrimaryAction) {
		KMessageBox::enableAllMessages();
		KSharedConfig::openConfig()->sync();	// Save the changes to disk.
	}
}

void KSudoku::changeEvent(QEvent *event)
{
	// Reimplemented to handle window minimize/maximize events
	if (event->type() == QEvent::WindowStateChange && !m_pauseButton->isChecked())
	{
		if (event->spontaneous() && !isMinimized() && currentGame().isValid())
		{
			// On maximize, set the game timer to the previous value
			m_gameUI->game().setTime(m_gameTime);
		}
		else if (event->spontaneous() && isMinimized() && currentGame().isValid())
		{
			// On minimize, capture the current game timer value
			m_gameTime = m_gameUI->game().msecsElapsed();
		}
	}
}

void KSudoku::setTimerDisplay()
{
	if(currentGame().isValid())
	{
		m_timeDisplay->setText(currentGame().time().toString(QStringLiteral("hh:mm:ss")));
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

#include "moc_ksudoku.cpp"
