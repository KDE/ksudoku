/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2007 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#include "ksudokugame.h"
#include "ksudoku_logging.h"

#include "puzzle.h"


#include "globals.h"

#include <KMessageBox>
#include <KLocalizedString>

#include <QList>
#include <QElapsedTimer>
#include <QTime>

class QWidget;

namespace ksudoku {
	
/**
 * @TODO replace m_refCount with QAtomic (in KDE4 version)
 */
class Game::Private : public GameIFace {
public:
	inline Private() : m_refCount(1) { }
	inline ~Private() {
		delete puzzle;
	}
public:
	inline void ref() { ++m_refCount; }
	inline bool deref() { return !--m_refCount; }
private:
	int m_refCount;
	
public: // The slots of GameIFace
	void undo() override;
	void redo() override;
	void addCheckpoint() override;
	void undo2Checkpoint() override;
	
public:
	inline void emitModified(bool isModified) { Q_EMIT modified(isModified); }
	inline void emitCompleted(bool isCorrect, const QTime& required, bool withHelp) {
		Q_EMIT completed(isCorrect, required, withHelp);
	}
	inline void emitCellChange(int index) { Q_EMIT cellChange(index); }
	inline void emitFullChange() { Q_EMIT fullChange(); }
	inline void emitCageChange(int cageNumP1, bool showLabel)
                                  { Q_EMIT cageChange(cageNumP1, showLabel); }
	
public:
	PuzzleState state;
	
	bool hadHelp : 1;
	bool wasFinished : 1;
	
	Puzzle* puzzle;
	QElapsedTimer time;
	int   accumTime;
	QUrl url;
	QList<HistoryEvent> history;
	int historyPos;

	QVector<int>  m_cage;
	int           m_cageValue;
	CageOperator  m_cageOperator;
	int           m_currentCageSaved;
	QWidget *     m_messageParent;
};

void Game::Private::undo() {
	if(historyPos == 0) return;
	
	HistoryEvent event(history[--historyPos]);
	event.undoOn(state);
	
	const QVector<int>& indices = event.cellIndices();
	if(indices.count() > 10) {
		Q_EMIT fullChange();
	} else {
		for(int i = 0; i < indices.count(); ++i)
			Q_EMIT cellChange(indices[i]);
	}
	Q_EMIT modified(true);
}

void Game::Private::redo() {
	if(historyPos == history.count()) return;
	
	HistoryEvent event(history[historyPos++]);
	event.redoOn(state);

	const QVector<int>& indices = event.cellIndices();
	if(indices.count() > 10) {
		Q_EMIT fullChange();
	} else {
		for(int i = 0; i < indices.count(); ++i)
			Q_EMIT cellChange(indices[i]);
	}
	Q_EMIT modified(true);
}

void Game::Private::addCheckpoint() {
}

void Game::Private::undo2Checkpoint() {
}

/*
 * The Game
 */

Game::Game()
	: m_private(0)
{
}

Game::Game(Puzzle* puzzle)
	: m_private(0)
{
	if(!puzzle) return;

	m_private = new Private();
	
	m_private->puzzle = puzzle;
	
	m_private->hadHelp = false;
	m_private->wasFinished = false;
	
	m_private->state = PuzzleState(size(), m_private->puzzle->order());
	m_private->state.reset();

	for(int i = 0; i < size(); i++) {
		m_private->state.setValue(i, m_private->puzzle->value(i));
		if(value(i) != 0)
			m_private->state.setGiven(i, true);
	}
	m_private->historyPos = 0;
	
	m_private->accumTime = 0;
	m_private->time.start();

	m_private->m_currentCageSaved = false;
}

Game::Game(const Game& game)
	: m_private(game.m_private)
{
	if(m_private) m_private->ref();
}

Game::~Game()
{
	if(m_private && m_private->deref()) delete m_private;
}

Game& Game::operator=(const Game& game) {
	if(m_private == game.m_private) return *this;
	
	if(m_private && m_private->deref()) delete m_private;
	m_private = game.m_private;
	if(m_private) game.m_private->ref();
	return *this;
}



bool Game::simpleCheck() const { // IDW TODO - This does nothing useful now
                                 //            that connections have gone.
	if(!m_private) return false;

	qCDebug(KSudokuLog) << "BYPASSED Game::simpleCheck()";
	return true; // IDW: disabled rest of test.

// IDW test. Eliminated optimized[] arrays and xxxConnection() functions.
}

void Game::restart() {
	while (canUndo()) {
		interface()->undo();
	}
	m_private->history.clear(); // otherwise we could do redo
	m_private->wasFinished = false;
	m_private->emitModified(true); // e.g. to update undo/redo action state
}

int Game::order() const {
	if(!m_private) return 0;
	return m_private->puzzle->order();
}

int Game::size() const {
	if(!m_private) return 0;
	return m_private->puzzle->size();
}

GameIFace* Game::interface() const {
	return m_private;
}

Puzzle* Game::puzzle() const {
	if(!m_private) return 0;
	return m_private->puzzle;
}

void Game::setUrl(const QUrl& url) {
	if(!m_private) return;
	
	m_private->url = url;
}

QUrl Game::getUrl() const {
	if(!m_private) return QUrl();
	return m_private->url;
}


void Game::setGiven(int index, bool given) {
	if(!m_private) return;
	
	if(given != m_private->state.given(index)) {
		if(given) {
			doEvent(HistoryEvent(index, CellInfo(GivenValue, m_private->state.value(index))));
		} else {
			doEvent(HistoryEvent(index, CellInfo(CorrectValue, m_private->state.value(index))));
		}
		m_private->emitCellChange(index);
		m_private->emitModified(true);
	}
}

bool Game::setMarker(int index, int val, bool state) {
	if(!m_private) return false;
	
	if(val == 0 || val > m_private->puzzle->order())
		return false;

	if(m_private->state.given(index))
		return false;
	int val2 = value(index);
	if(val == val2) {
		doEvent(HistoryEvent(index, CellInfo()));
	} else {
		QBitArray markers = m_private->state.markers(index);
		markers.detach();
		if(val2 != 0) {
			markers.setBit(val2 - 1, true);
		}
		markers.setBit(val - 1, state);
		doEvent(HistoryEvent(index, CellInfo(markers)));
	}
	
	// almost every time this function will change the cell
	m_private->emitCellChange(index);
	m_private->emitModified(true);
	
	return true;
}

void Game::setValue(int index, int val) {
	if(!m_private) return;
	// If entering in a puzzle, Mathdoku/KillerSudoku has its own procedure.
	if (! m_private->puzzle->hasSolution()) {
	    if (addToCage (index, val)) {
		return;		// Value went in a Mathdoku/KillerSudoku puzzle.
	    }
	}
	if ((val == 32) || (val == 26)) {	// Delete-action or Qt::Key_0.
	    val = 0;				// Clear the cell.
	}

	// Solve all kinds of puzzles or enter in a Sudoku or Roxdoku puzzle.
	if(val > m_private->puzzle->order()) return;
	
	if(m_private->state.given(index)) return;
	
	int oldvalue = value(index);
	doEvent(HistoryEvent(index, CellInfo(CorrectValue, val)));
	
	m_private->emitCellChange(index);
	m_private->emitModified(true);

	if(oldvalue != val)
		checkCompleted();
}

bool Game::addToCage (int pos, int val)
{
	SKGraph * g = m_private->puzzle->graph();
	SudokuType t = g->specificType();
	if ((t != Mathdoku) && (t != KillerSudoku)) {
	    return false;	// We are not keying in a cage.
	}
#ifdef MATHDOKUENTRY_LOG
	qCDebug(KSudokuLog) << "Game::addToCage: pos" << pos << "action" << val;
#endif
	if (! m_private->m_currentCageSaved) {	// Start a new cage.
	    m_private->m_cage.clear();
	    m_private->m_cageValue = 0;
	    m_private->m_cageOperator = NoOperator;
	}
	if ((val != 32) && (! validCell (pos, g))) {
	    return true;	// Invalid pos and not deleting: go no further.
	}
	CageOperator cageOp = m_private->m_cageOperator;
	if ((val >= 1) && (val <= 9)) {
	    // Append a non-zero digit to the cage-value.
	    m_private->m_cageValue = 10 * m_private->m_cageValue + val;
	}
	else {
	    switch (val) {
	    case 24:	// Qt::Key_X = multiply.
		cageOp = Multiply;
		break;
	    case 26:	// Qt::Key_0
		if (m_private->m_cageValue > 0) {
		    // Append a zero to the cage-value.
		    m_private->m_cageValue = 10 * m_private->m_cageValue;
		}
		break;
	    case 27:	// Qt::Key_Slash.
		cageOp = Divide;
		break;
	    case 28:	// Qt::Key_Minus.
		cageOp = Subtract;
		break;
	    case 29:	// Qt::Key_Plus.
		cageOp = Add;
		break;
	    case 30:	// Left click or Qt::Key_Space = drop through and
		break;	//                               add cell to cage.
	    case 31:	// Qt::Key_Return = end cage.
		finishCurrentCage (g);
		return true;
		break;
	    case 32:	// Right click or Delete/Bkspace = delete a whole cage.
		deleteCageAt (pos, g);
		return true;
		break;
	    default:
		return false;
		break;
	    }
	}

	// Valid keystroke and position: store and display the current cage.
	if (m_private->m_cage.indexOf (pos) < 0) {
	    m_private->m_cage.append (pos);	// Add cell to current cage.
	}
	if (t == KillerSudoku) {
	    if (cageOp != NoOperator) {
		KMessageBox::information (messageParent(),
		    i18n("In Killer Sudoku, the operator is always + or none "
			 "and KSudoku automatically sets the correct choice."),
		    i18n("Killer Sudoku Cage"), QStringLiteral("KillerCageInfo"));
	    }
	    // Set the operator to none or Add, depending on the cage-size.
	    cageOp = (m_private->m_cage.size() > 1) ?  Add : NoOperator;
	}
	// TODO - In Killer Sudoku, show the operator during data-entry.
	m_private->m_cageOperator = cageOp;

	// Change the last cage in the data-model in the SKGraph object.
	if (m_private->m_currentCageSaved) {	// If new cage, skip dropping.
	    int cageNum = g->cageCount() - 1;
#ifdef MATHDOKUENTRY_LOG
	    qCDebug(KSudokuLog) << "    DROPPING CAGE" << cageNum
		     << "m_currentCageSaved" << m_private->m_currentCageSaved
		     << "m_cage" << m_private->m_cage;
#endif
	    g->dropCage (cageNum);
	}
	// Add a new cage or replace the previous version of the new cage.
	g->addCage (m_private->m_cage,
		    m_private->m_cageOperator, m_private->m_cageValue);
#ifdef MATHDOKUENTRY_LOG
	qCDebug(KSudokuLog) << "    ADDED CAGE" << (g->cageCount() - 1)
		 << "value" << m_private->m_cageValue
		 << "op" << m_private->m_cageOperator
		 << m_private->m_cage;
#endif
	m_private->m_currentCageSaved = true;

	// Re-draw the boundary and label of the cage just added to the graph.
	// We always display the label while the cage is being keyed in.
	m_private->emitCageChange (g->cageCount(), true);
	return true;
}

bool Game::validCell (int pos, SKGraph * g)
{
	// No checks of selected cell needed if it is in the current cage.
	if (m_private->m_cage.indexOf (pos) >= 0) {
	    return true;
	}
	// Selected cell must not be already in another cage.
	for (int n = 0; n < g->cageCount(); n++) {
	    if (g->cage(n).indexOf (pos) >= 0) {
		KMessageBox::information (messageParent(),
		    i18n("The cell you have selected has already been "
			 "used in a cage."),
		    i18n("Error in Cage"));
		return false;
	    }
	}
	// Cell must adjoin the current cage or be the first cell in it.
	int cageSize = m_private->m_cage.size();
	if (cageSize > 0) {
	    int  ix = g->cellPosX(pos);
	    int  iy = g->cellPosY(pos);
	    int  max = g->order();
	    bool adjoining = false;
	    for (int n = 0; n < cageSize; n++) {
		int cell = m_private->m_cage.at(n);
		int dx = g->cellPosX(cell) - ix;
		int dy = g->cellPosY(cell) - iy;
		if ((dy == 0) && (((ix > 0) && (dx == -1)) ||
				  ((ix < max) && (dx == 1)))) {
		    adjoining = true;	// Adjoining to left or right.
		    break;
		}
		if ((dx == 0) && (((iy > 0) && (dy == -1)) ||
				  ((iy < max) && (dy == 1)))) {
		    adjoining = true;	// Adjoining above or below.
		    break;
		}
	    }
	    if (! adjoining) {
		KMessageBox::information (messageParent(),
		    i18n("The cell you have selected is not next to "
			 "any cell in the cage you are creating."),
		    i18n("Error in Cage"));
		return false;
	    }
	}
	return true;
}

void Game::finishCurrentCage (SKGraph * g)
{
#ifdef MATHDOKUENTRY_LOG
	qCDebug(KSudokuLog) << "END CAGE: value" << m_private->m_cageValue
		 << "op" << m_private->m_cageOperator
		 << m_private->m_cage;
#endif
	// If Killer Sudoku and cage-size > 1, force operator to be +.
	if ((g->specificType() == KillerSudoku) &&
	    (m_private->m_cage.size() > 1)) {
	    m_private->m_cageOperator = Add;
	}
	// Validate the contents of the cage.
	if ((! m_private->m_currentCageSaved) ||
	    (m_private->m_cage.size() == 0)) {
	    KMessageBox::information (messageParent(),
		i18n("The cage you wish to complete has no cells in it yet. "
		     "Please click on a cell or key in + - / x or a number."),
		i18n("Error in Cage"));
	    return;		// Invalid - cannot finalise the cage.
	}
	else if (m_private->m_cageValue == 0) {
	    KMessageBox::information (messageParent(),
		i18n("The cage you wish to complete has no value yet. "
		     "Please key in a number with one or more digits."),
		i18n("Error in Cage"));
	    return;		// Invalid - cannot finalise the cage.
	}
	else if ((m_private->m_cage.size() > 1) &&
	         (m_private->m_cageOperator == NoOperator)) {
	    KMessageBox::information (messageParent(),
		i18n("The cage you wish to complete has more than one cell, "
		     "but it has no operator yet. Please key in + - / or x."),
		i18n("Error in Cage"));
	    return;		// Invalid - cannot finalise the cage.
	}
	else if ((m_private->m_cage.size() == 1) &&
	         (m_private->m_cageValue > g->order())) {
	    KMessageBox::information (messageParent(),
		i18n("The cage you wish to complete has one cell, but its "
		     "value is too large. A single-cell cage must have a value "
		     "from 1 to %1 in a puzzle of this size.", g->order()),
		i18n("Error in Cage"));
	    return;		// Invalid - cannot finalise the cage.
	}

	// Save and display the completed cage.
	if (m_private->m_cage.size() == 1) {	// Display digit.
	    doEvent(HistoryEvent(m_private->m_cage.first(),
		    CellInfo(CorrectValue, m_private->m_cageValue)));
	    m_private->emitCellChange(m_private->m_cage.first());
	    m_private->emitModified(true);
	}
	// IDW TODO - Unhighlight the cage that is being entered.
	m_private->emitCageChange (g->cageCount(),	// No label in size 1.
				   (m_private->m_cage.size() > 1));
	// Start a new cage.
	m_private->m_currentCageSaved = false;
}

void Game::deleteCageAt (int pos, SKGraph * g)
{
	int cageNumP1 = g->cageCount();
	if (cageNumP1 > 0) {
	    // IDW TODO - Hover-hilite the cage that is to be deleted.
	    cageNumP1 = 0;
	    for (int n = 0; n < g->cageCount(); n++) {
		if (g->cage(n).indexOf (pos) >= 0) {
		    cageNumP1 = n + 1;		// This cage is to be deleted.
		    break;
		}
	    }
	    // If the right-click was on a cage, delete it.
	    if (cageNumP1 > 0) {
		if(KMessageBox::questionYesNo (messageParent(),
		       i18n("Do you wish to delete this cage?"),
		       i18n("Delete Cage"), KStandardGuiItem::del(),
		       KStandardGuiItem::cancel(), QStringLiteral("CageDelConfirm"))
		       == KMessageBox::No) {
		    return;
		}
		if (g->cage(cageNumP1-1).size() == 1) {	// Erase digit.
		    // Delete the digit shown in a size-1 cage.
		    doEvent(HistoryEvent(pos, CellInfo(CorrectValue, 0)));
		    m_private->emitCellChange(pos);
		    m_private->emitModified(true);
		}
		// Erase the cage boundary and label.
		m_private->emitCageChange (-cageNumP1, false);
		// Remove the cage from the puzzle's graph.
#ifdef MATHDOKUENTRY_LOG
		qCDebug(KSudokuLog) << "    DROP CAGE" << (cageNumP1 - 1);
#endif
		g->dropCage (cageNumP1 - 1);
		if (m_private->m_cage.indexOf (pos) >= 0) {
		    // The current cage was dropped.
		    m_private->m_currentCageSaved = false;
		}
	    }
	    else {
		KMessageBox::information (messageParent(),
		    i18n("The cell you have selected is not in any cage, "
			 "so the Delete action will not delete anything."),
		    i18n("Delete Cage"), QStringLiteral("CageDelMissed"));
	    }
	}
	else {
	    KMessageBox::information (messageParent(),
		i18n("The Delete action finds that there are no cages "
		     "to delete."), i18n("Delete Cage"));
#ifdef MATHDOKUENTRY_LOG
	    qCDebug(KSudokuLog) << "NO CAGES TO DELETE.";
#endif
	}
}

bool Game::allValuesSetAndUsable() const {
	for (int i = 0; i < size(); i++) {
		if (value(i) == 0) {
			return false;
		}
	}

	return true;
}

void Game::checkCompleted() {
	if(!m_private || !m_private->puzzle->hasSolution()) return;

	if (!allValuesSetAndUsable()) {
		return;
	}

	for(int i = 0; i < size(); i++) {
		if(value(i) != solution(i)) {
			m_private->emitCompleted(false, time(), m_private->hadHelp);
			return;
		}
	}
	m_private->wasFinished = true;
	m_private->emitCompleted(true, time(), m_private->hadHelp);
}

bool Game::giveHint() {
	if(!m_private || !m_private->puzzle->hasSolution()) return false;
	
	int moveNum = 0;
	int index = 0;
	while (true) {
		index = m_private->puzzle->hintIndex(moveNum);
		if (index < 0) {
			return false;	// End of hint-list.
		}
		if (value(index) == 0) {
			break;		// Hint is for a cell not yet filled.
		}
		moveNum++;
	}

	m_private->hadHelp = true;
	
	int val = solution(index);
	doEvent(HistoryEvent(index, CellInfo(GivenValue, val)));
	
	m_private->emitCellChange(index);
	m_private->emitModified(true);
	
	checkCompleted();
	
	return true;
}

bool Game::autoSolve() {
	if(!m_private || !m_private->puzzle->hasSolution()) return false;
	
	m_private->hadHelp = true;
	
	PuzzleState newState(size(), m_private->puzzle->order());
	newState.reset();
	
	for(int i = 0; i < size(); ++i) {
		int val = solution(i);
		newState.setValue(i, val);
		newState.setGiven(i, true);
	}
	
	doEvent(HistoryEvent(newState));
	
	m_private->emitFullChange();
	m_private->emitModified(true);

	m_private->wasFinished = true;
	m_private->emitCompleted(true, time(), true);
	
	return true;
}


int Game::value(int index) const {
	if(!m_private) return 0;
	return m_private->state.value(index);
}

int Game::solution(int index) const {
	if(!m_private) return 0;
	return m_private->puzzle->solution(index);
}

bool Game::given(int index) const {
	if(!m_private) return false;
	return m_private->state.given(index);
}

bool Game::marker(int index, int val) const {
	if(!m_private) return false;
	return m_private->state.marker(index, val);
}

ksudoku::ButtonState Game::buttonState(int index) const {
	if(!m_private) return WrongValue;
	
	if(given(index))
		return GivenValue;
	if(value(index) == 0)
		return Marker;
	if(value(index) == solution(index))
		return CorrectValue;
	if(solution(index))
		return WrongValue;
	return CorrectValue;
}

CellInfo Game::cellInfo(int index) const {
	if(!m_private)
		return CellInfo(WrongValue, 0);
				
	if(given(index))
		return CellInfo(GivenValue, value(index));
	if(value(index) == 0)
		return CellInfo(m_private->state.markers(index));
	if(value(index) == solution(index))
		return CellInfo(CorrectValue, value(index));
	if(solution(index))
		return CellInfo(WrongValue, value(index));
	return CellInfo(CorrectValue, value(index));
}

const BoardContents Game::allValues() const {
	if(!m_private) return BoardContents();
	
	return m_private->state.allValues();
}

QTime Game::time() const {
	if(!m_private) return QTime();
	return QTime(0,0).addMSecs(msecsElapsed());
}

int Game::msecsElapsed() const {
	if(!m_private) return 0;
	return (m_private->accumTime + m_private->time.elapsed());
}

void Game::setTime(int msecs) const {
	if(!m_private) return;
	m_private->accumTime = msecs;
	m_private->time.start();
}

// History

void Game::doEvent(const HistoryEvent& event) {
	if(!m_private) return;
	
	HistoryEvent hisEvent(event);
	
	// Remove events after current history position
	m_private->history.erase(m_private->history.begin()+(m_private->historyPos), m_private->history.end());
	
	// Append event
	hisEvent.applyTo(m_private->state);
	m_private->history.append(hisEvent); // always append after applying
	m_private->historyPos++;
}

int Game::historyLength() const {
	if(!m_private) return 0;
	
	return m_private->history.count();
}

HistoryEvent Game::historyEvent(int i) const {
	if(!m_private || i >= m_private->history.count())
		return HistoryEvent();
	
	return m_private->history[i];
}

bool Game::canUndo() const {
	if(!m_private) return false;
	
	return m_private->historyPos != 0;
}

bool Game::canRedo() const {
	if(!m_private) return false;
	return m_private->historyPos != m_private->history.count();
}

bool Game::userHadHelp() const {
	if(!m_private) return false;
	return m_private->hadHelp;
}

bool Game::wasFinished() const {
	if(!m_private) return false;
	return m_private->wasFinished;
}

void Game::setUserHadHelp(bool hadHelp) {
	if(!m_private) return;
	m_private->hadHelp = hadHelp;
}

void Game::setMessageParent(QWidget * messageParent)
{
	if (m_private) {
	    m_private->m_messageParent = messageParent;
	}
}

QWidget * Game::messageParent()
{
	return (m_private ? m_private->m_messageParent : 0);
}

}


