/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2007 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
 *   Copyright 2012      Ian Wadham <iandw.au@gmail.com>                   *
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

#ifndef _KSUDOKU_H_
#define _KSUDOKU_H_

#include <QUrl>
#include <QTime>
#include <QTimer>
#include <QLabel>
#include <KXmlGuiWindow>
#if 0
#include <knewstuff/knewstuff.h>
#endif

class KGameDifficultyLevel;

namespace ksudoku {
class KsView;
class GameActions;
class GameVariantCollection;
class ValueListWidget;
class WelcomeScreen;
class Game;
class Puzzle;
}

class PuzzlePrinter;

/**
 * This class serves as the main window for ksudoku.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Francesco Rossi <redsh@email.it>
 * @version 0.3
 */

class KSudoku;

#if 0
class KSudokuNewStuff : public KNewStuff
{
public:
	KSudoku* parent;
public:
	KSudokuNewStuff(KSudoku* p);
	bool install( const QString &fileName );
	bool createUploadFile( const QString &fileName );
};
#endif

class KSudoku : public KXmlGuiWindow
{
    Q_OBJECT

public:
    /**
     * Default Constructor
     */
	KSudoku();

    /**
     * Default Destructor
     */
	~KSudoku() override;

	void loadGame(const QUrl& url);

public:
	void updateShapesList();

	ksudoku::Game    currentGame() const;
	ksudoku::KsView* currentView() const;
protected:
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dropEvent(QDropEvent *event) override;

public Q_SLOTS:
    void onCompleted(bool isCorrect, QTime required, bool withHelp = false);

	void showWelcomeScreen();

	void startGame(const ::ksudoku::Game& game);
	void endCurrentGame();

	bool queryClose() override;

private Q_SLOTS:
	void handleCurrentDifficultyLevelChanged(const KGameDifficultyLevel *level);
	void symmetryChanged   (int symmetry);

	void gameNew();
	void gameRestart();
	void gameOpen();
	void gameSave();
	void gameSaveAs();
	void gamePrint();
	void gameExport();
	void gamePause();

	void undo();
	void redo();
	void push();
	void pop();

	void giveHint();
	void autoSolve();
	void dubPuzzle();
	void genMultiple();

	void optionsPreferences();
	void updateSettings();
	void changeCaption(const QString& text);
//  void updateStatusBar();
//  void changeStatusbar(const QString& text);

	void onModified(bool isModified);

	void enableMessages();

	/*
	 * Can be reimplemented per
	 * https://doc.qt.io/qt-6/qwidget.html#changeEvent
	 */
	void changeEvent(QEvent *event) override;
	void setTimerDisplay();

Q_SIGNALS:
	void settingsChanged();

private:
	void setupActions();
	void setupStatusBar(int difficulty, int symmetry);

	void adaptActions2View();

private:
	QWidget* wrapper;

	QAction * m_pauseButton;
	QAction* m_gameSave;
	QAction* m_gameSaveAs;
	int m_gameTime;
	ksudoku::GameVariantCollection* m_gameVariants;
	ksudoku::WelcomeScreen* m_welcomeScreen;

	QWidget* m_gameWidget;
	ksudoku::ValueListWidget* m_valueListWidget;

	ksudoku::KsView* m_gameUI;

	ksudoku::GameActions* m_gameActions;

	PuzzlePrinter * m_puzzlePrinter;

	QLabel *m_timeDisplay;
	QTimer *m_timer;

	// Indicates the pause button has been pressed at least once
	// and flagging message queue upon solving
	// see KSudoku::onCompleted
	bool m_wasPaused;
};

#endif // _KSUDOKU_H_

