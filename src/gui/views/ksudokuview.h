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

#ifndef _KSUDOKUVIEW_H_
#define _KSUDOKUVIEW_H_

#include "ksview.h"


//#include "ksudokuiface.h"
#include "qsudokubutton.h"
//Added by qt3to4:
#include <QResizeEvent>

#include "ksudokugame.h"


using namespace ksudoku;


class QPainter;


namespace ksudoku {

class QSudokuButton;
class Symbols;


/**
 * Gui for a sudoku puzzle
 * @TODO rename ksudokuView to sudokuView
 */
class ksudokuView : public QWidget
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
	void mouseOverCell(int cell);
	void valueSelected(int value);

public:
	bool mouseOnlySuperscript;
	bool showTracker;
	int  isWaitingForNumber;
	int current_selected_number;

	bool custom;
	
	
	Game game() { return m_game; }
	QChar symbol(int value) const;
public slots:
	void setCursor(int cell);
	void selectValue(int value);
	void setSymbols(SymbolTable* table);
	void setFlags(ViewFlags flags);
	
	/// call this to update the view
	/// if @p cell is a positive value only the according
	/// cell will be updated
	void update(int cell = -1);
	
protected:
	void resizeEvent(QResizeEvent *);
	void wheelEvent(QWheelEvent* e);
	
private slots:
	void slotHello(int x, int y);
	void btn_enter(int x, int y);
	void btn_leave(int x, int y);
	
	void slotRight(int x, int y);

	void beginHighlight(int val);
	void finishHighlight();

	
	QWidget* widget() { return this; }

private:
	void setGame(const Game& game);
	
	QVector<QSudokuButton*> m_buttons;
	
	int m_currentCell;
	
	QVector<int> m_highlightUpdate;

	bool puzzle_mark_wrong;
	int  highlighted;
	int m_color0;
	
	bool m_guidedMode;
	
	Game m_game;
	SymbolTable* m_symbolTable;
};

}

#endif // _KSUDOKUVIEW_H_
