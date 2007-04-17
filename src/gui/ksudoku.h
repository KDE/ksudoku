// part of KSUDOKU - by Francesco Rossi <redsh@email.it> 2005

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

#include "gameseldlg.h"

//#include "skgraph.h"
//#include "sksolver.h"

class ksudoku::KsView;
class KPrinter;
class KUrl;
class KTabWidget;

namespace ksudoku {
class GameSelectionDialog;
class GameOptionsDialog;
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
// 	/**
// 	 * Creates a new KSudoku-Window for a existing Game 
// 	 */
// 	KSudoku(ksudoku::Game* game);

    /**
     * Default Destructor
     */
	virtual ~KSudoku();
	
	void addGame(const Game& game);
	void newGame();
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
	void selectGameType(const QString& type);
	void startSelectedGame();

private slots:
	void dlgSelectedGame(const QString& name);
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

	void selectNumber(uint value);

	void set0();
	void set1();
	void set2();
	void set3();
	void set4();
	void set5();
	void set6();
	void set7();
	void set8();
	void set9();
	void set10();
	void set11();
	void set12();
	void set13();
	void set14();
	void set15();
	void set16();

	void set17();
	void set18();
	void set19();
	void set20();
	void set21();
	void set22();
	void set23();
	void set24();
	void set25();

	void optionsPreferences();
	void setShowTracker();
	void changeStatusbar(const QString& text);
	void changeCaption(const QString& text);

	//web
	void checkForUpdates();
	void homepage();
	void support();
	void sendComment();
	//settings
	void mouseOnlySuperscript();
	void setGuidedMode();

	void updateStatusBar();

	void onModified(bool isModified);
	
public:
	void updateCustomShapesList();
	void loadCustomShapeFromPath();
	void createCustomShape();
	QString getShapeName(QString path);

private:
	QWidget* wrapper;
	QWidget* activeWidget;
	void KDE3Action(QString text, QWidget* object, const char* slot, QString name);
	void setupAccel();
	void setupActions();

	void adaptActions2View();

	
private:
// 	KTabWidget* m_tabs;
	
	GameSelectionDialog* m_gameSelDlg;
	
	bool m_autoDelCentralWidget;
	
	QString m_defaultAction;
	GameOptionsDialog* m_gameOptionsDlg;
	bool m_optionEnterOwnGame;
	
	QMap<QString, SKSolver*> m_shapes;
	QStringList m_shapes_paths;
	QString m_shape_save_path;
};

#endif // _KSUDOKU_H_

