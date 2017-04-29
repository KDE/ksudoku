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

#include <kxmlguiwindow.h>
#if 0
#include <knewstuff/knewstuff.h>
#endif

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgamedifficulty.h>

class QPrinter;

namespace ksudoku {
class KsView;
class GameActions;
class GameVariantCollection;
class ValueListWidget;
class WelcomeScreen;
class Game;
class Puzzle;
}

class SKGraph;
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
	virtual ~KSudoku();
	
	void loadGame(const QUrl& url);
	
public:
	void updateShapesList();
	void createCustomShape();

	ksudoku::Game    currentGame() const;
	ksudoku::KsView* currentView() const;

protected:
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dropEvent(QDropEvent *event);

public slots:
	void onCompleted(bool isCorrect, const QTime& required, bool withHelp = false);
	
	void showWelcomeScreen();

	void startGame(const ::ksudoku::Game& game);
	void endCurrentGame();

	bool queryClose();
	
private slots:
	void difficultyChanged (KGameDifficulty::standardLevel difficulty);
	void difficultyChanged (int difficulty); // IDW test.
	void symmetryChanged   (int symmetry);

	void gameNew();
	void gameOpen();
	void gameSave();
	void gameSaveAs();
	void gamePrint();
	void gameExport();

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

	void homepage();


	void onModified(bool isModified);

	void enableMessages();

signals:
	void settingsChanged();

private:
	void setupActions();
	void setupStatusBar(int difficulty, int symmetry);

	void adaptActions2View();

private:
	QWidget* wrapper;
	
	QAction* m_gameSave;
	QAction* m_gameSaveAs;

	ksudoku::GameVariantCollection* m_gameVariants;
	ksudoku::WelcomeScreen* m_welcomeScreen;
	
	QWidget* m_gameWidget;
	ksudoku::ValueListWidget* m_valueListWidget;
	
	ksudoku::KsView* m_gameUI;

	ksudoku::GameActions* m_gameActions;

	PuzzlePrinter * m_puzzlePrinter;
};

#endif // _KSUDOKU_H_

