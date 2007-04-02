// part of KSUDOKU

#ifndef _KSUDOKUVIEW_H_
#define _KSUDOKUVIEW_H_

#include "ksview.h"


//#include "ksudokuiface.h"
#include "qsudokubutton.h"
#include <q3valuestack.h>
#include <q3ptrvector.h>
//Added by qt3to4:
#include <QResizeEvent>

#include "ksudokugame.h"


using namespace ksudoku;


class QPainter;
class KURL;


namespace ksudoku {

class QSudokuButton;


/**
 * Gui for a sudoku puzzle
 * @TODO rename ksudokuView to sudokuView
 */
class ksudokuView : public QWidget, public KsView
{
	Q_OBJECT
	friend class QSudokuButton;
public:
	/// Default constructor
	ksudokuView(QWidget *parent, const Game& game, bool custom);
	// Destructor
	virtual ~ksudokuView();

	virtual QString status() const;
	int getHighlighted(){return highlighted;}


	/**
	 * Draw view to external QPainter device
	 * QPainter should be open en will be left open
	 * (implemented from KsView)
	 * 
	 * @TODO hide private members
	 */
	virtual void draw(QPainter& p, int height, int width) const;

signals:
	void changedSelectedNum();

public:
// 	void setup  (const Game& game);

	bool mouseOnlySuperscript;
	bool showTracker;
	int  isWaitingForNumber;
	int current_selected_number;

	bool custom;

protected:
	void resizeEvent(QResizeEvent *);
	//void paintEvent(QPaintEvent *);

private slots:
	void slotHello(int x, int y);
	void btn_enter(int x, int y);
	void btn_leave(int x, int y);
	
	void slotRight(int x, int y);

	void beginHighlight(int val);
	void finishHighlight();

	void onCellChange(int index);
	void onFullChange();
	
	QWidget* widget() { return this; }

private:
	void setGame(const Game& game);
	
	Q3PtrVector<QSudokuButton> m_buttons;

	bool puzzle_mark_wrong;
	int  highlighted;
	int m_color0;
};

}

#endif // _KSUDOKUVIEW_H_
