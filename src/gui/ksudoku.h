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

#ifndef _KSUDOKU_H_
#define _KSUDOKU_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kxmlguiwindow.h>
#include <qdatetime.h>
//Added by qt3to4:
#include <QDropEvent>
#include <QDragEnterEvent>
#include <kurl.h>
#if 0
#include <knewstuff/knewstuff.h>
#endif

#include "ksudokuview.h"
#include "roxdokuview.h"

#include "symbols.h"

#include <QSignalMapper>

#include <QListWidget>

//#include "skgraph.h"
//#include "sksolver.h"

class ksudoku::KsView;
class KPrinter;
class KUrl;
class KTabWidget;

namespace ksudoku {
class GameSelectionDialog;
class GameOptionsDialog;
class GameVariantCollection;
class ValueListWidget;
class WelcomeScreen;
}

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
	
	void loadGame(const KUrl& url);
	
	Game             currentGame() const;
	ksudoku::KsView* currentView() const;

	// override central Widget 
	virtual void setCentralWidget(QWidget* widget) { setCentralWidget(widget, false); }
	void setCentralWidget(QWidget* widget, bool autoDel);

	QMap<QString, SKSolver*>& shapes(){return m_shapes;}

protected:
    /**
     * Overridden virtuals for Qt drag 'n drop (XDND)
     */
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dropEvent(QDropEvent *event);

protected:
    /**
     * This function is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfig *);

    /**
     * This function is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(KConfig *);
	
public slots:
	void onCompleted(bool isCorrect, const QTime& required, bool withHelp = false);
	
	void showWelcomeScreen();

	void startGame(const Game& game);
	
private slots:
	void fileNew();
	void fileOpen();
	void fileSave();
	void fileSaveAs();
	void filePrint();
	void fileExport();

	void undo();
	void redo();
	void push();
	void pop();

	void giveHint();
	void autoSolve();
	void dubPuzzle();
	void genMultiple();

	void selectValue(int value);
	void enterValue(int value);
	void markValue(int value);
	void moveUp();
	void moveDown();
	void moveLeft();
	void moveRight();
	void clearCell();

	void optionsPreferences();
	void settingsChanged();
	void setShowTracker();
	void changeStatusbar(const QString& text);
	void changeCaption(const QString& text);

	//web
	void homepage();
	void support();
	void sendComment();
	//settings
	void mouseOnlySuperscript();
	void setGuidedMode();

	void updateStatusBar();

	void onModified(bool isModified);
	
public:
	void updateShapesList();
	void loadCustomShapeFromPath();
	void createCustomShape();

private:
	QWidget* wrapper;
	QWidget* activeWidget;
	void KDE3Action(const QString& text, QWidget* object, const char* slot, const QString& name);
	void setupAccel();
	void setupActions();

	void adaptActions2View();

	
private:
// 	KTabWidget* m_tabs;
	
// 	GameSelectionDialog* m_gameSelDlg;
	
	QSignalMapper* m_selectValueMapper;
	QSignalMapper* m_enterValueMapper;
	QSignalMapper* m_markValueMapper;
	
	QAction* m_moveUpAct;
	QAction* m_moveDownAct;
	QAction* m_moveLeftAct;
	QAction* m_moveRightAct;
	
	GameVariantCollection* m_gameVariants;
	WelcomeScreen* m_welcomeScreen;
	
	QWidget* m_gameWidget;
	ValueListWidget* m_valueListWidget;

	bool m_autoDelCentralWidget;
	
	QString m_defaultAction;
	bool m_optionEnterOwnGame;
	
	QMap<QString, SKSolver*> m_shapes;
	QStringList m_shapes_paths;
	QString m_shape_save_path;
	
	Symbols m_symbols;
};

#endif // _KSUDOKU_H_

