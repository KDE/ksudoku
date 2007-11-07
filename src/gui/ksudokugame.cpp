/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
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

#include "ksudokugame.h"

#include "sudoku_solver.h"

#include "puzzle.h"

#include "history.h"

//Added by qt3to4:
#include <QList>

#include <kurl.h>


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
	void undo();
	void redo();
	void addCheckpoint();
	void undo2Checkpoint();
	
public:
	inline void emitModified(bool isModified) { emit modified(isModified); }
	inline void emitCompleted(bool isCorrect, const QTime& required, bool withHelp) {
		emit completed(isCorrect, required, withHelp);
	}
	inline void emitCellChange(int index) { emit cellChange(index); }
	inline void emitFullChange() { emit fullChange(); }
	
public:
	PuzzleState state;
	
	bool hadHelp: 1;
	
	Puzzle* puzzle;
	QTime time;
	KUrl url;
	QList<HistoryEvent> history;
	int historyPos;
};

void Game::Private::undo() {
	if(historyPos == 0) return;
	
	HistoryEvent event(history[--historyPos]);
	event.undoOn(state);
	
	const QVector<int>& indices = event.cellIndices();
	if(indices.count() > 10) {
		emit fullChange();
	} else {
		for(int i = 0; i < indices.count(); ++i)
			emit cellChange(indices[i]);
	}
	emit modified(true);
}

void Game::Private::redo() {
	if(historyPos == history.count()) return;
	
	HistoryEvent event(history[historyPos++]);
	event.redoOn(state);

	const QVector<int>& indices = event.cellIndices();
	if(indices.count() > 10) {
		emit fullChange();
	} else {
		for(int i = 0; i < indices.count(); ++i)
			emit cellChange(indices[i]);
	}
	emit modified(true);
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
	
	m_private->state = PuzzleState(size(), m_private->puzzle->order());
	m_private->state.reset();

	for(int i = 0; i < size(); i++) {
		m_private->state.setValue(i, m_private->puzzle->value(i));
		if(value(i) != 0)
			m_private->state.setGiven(i, true);
	}
	m_private->historyPos = 0;
	
	m_private->time.start();
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



bool Game::simpleCheck() const {
	if(!m_private) return false;
	
	for(int i = 0; i < size(); ++i) {
		if(m_private->puzzle->value(i) == 0)
			continue;
		
		for(int j = 0; j < m_private->puzzle->optimized_d(i); ++j){
			// set k to the index of the connected field
			int k = m_private->puzzle->optimized(i,j);
			if(k == i)
				continue;

			// change k to the value of the connected field
			k = m_private->puzzle->value(i);
			if(k == 0)
				continue;
			if(k == m_private->puzzle->value(i))
				return false;
		}
	}	
	return true;
}

int Game::order() const {
	if(!m_private) return 0;
	return m_private->puzzle->order();
}

int Game::index(int x, int y, int z) const {
	if(!m_private) return 0;
	return m_private->puzzle->index(x,y,z);
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

bool Game::hasSolver()
{
	if(!m_private)
		return 0;
	return m_private->puzzle->hasSolver();
}

void Game::setUrl(const KUrl& url) {
	if(!m_private) return;
	
	m_private->url = url;
}

KUrl Game::getUrl() const {
	if(!m_private) return KUrl();
	return m_private->url;
}


QBitArray Game::highlightValueConnections(int val, bool allValues) const {
	if(!m_private) return QBitArray();
		
	if(val <= 0 || val > m_private->puzzle->order())
		return QBitArray();
	
	QBitArray array(size());
	for(int i = 0; i < size(); i++)
		array.clearBit(i);
	
	for(int i = 0; i < size(); i++) {
		if(allValues && value(i))
			array.setBit(i);
		if(value(i) == val) {
			for(int j = 0; j < m_private->puzzle->optimized_d(i); j++)
				array.setBit(m_private->puzzle->optimized(i,j));
		}
	}

	return array;
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
	if(val > m_private->puzzle->order()) return;
	
	if(m_private->state.given(index)) return;
	
	int oldvalue = value(index);
	doEvent(HistoryEvent(index, CellInfo(CorrectValue, val)));
	
	m_private->emitCellChange(index);
	m_private->emitModified(true);

	if(oldvalue != val)
		checkCompleted();
}

void Game::checkCompleted() {
	if(!m_private || !m_private->puzzle->hasSolution()) return;

	for(int i = 0; i < size(); i++)
		if(value(i) == 0)
			return;
	
	for(int i = 0; i < size(); i++) {
		if(value(i) != m_private->puzzle->solution(i)) {
			m_private->emitCompleted(false, time(), m_private->hadHelp);
			return;
		}
	}
	m_private->emitCompleted(true, time(), m_private->hadHelp);
}

bool Game::giveHint() {
	if(!m_private || !m_private->puzzle->hasSolution()) return false;
	
	m_private->hadHelp = true;
	
// 	uint remaining = 0;
// 	for(uint i = 0; i < size(); ++i) {
// 		if(!isGiven(i)) remaining++;
// 	}
// 	if(!remaining) return false;
	
// 	int i;
// 	do {
// 		i = RANDOM(size());
// 	} while (m_private->isGiven(i));
	
	int start = RANDOM(size());
	int i;
	for(i = start; i < size(); ++i)
		if(!given(i))
			break;
	if(i == size()) {
		for(i = 0; i < start; ++i)
			if(!given(i))
				break;
		if(i == start) return false;
	}
	
	int val = m_private->puzzle->solution(i);
	doEvent(HistoryEvent(i, CellInfo(GivenValue, val)));
	
	m_private->emitCellChange(i);
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
		int val = m_private->puzzle->solution(i);
		newState.setValue(i, val);
		newState.setGiven(i, true);
	}
	
	doEvent(HistoryEvent(newState));
	
	m_private->emitFullChange();
	m_private->emitModified(true);
	
	m_private->emitCompleted(true, time(), true);
	
	return true;
}


int Game::value(int index) const {
	if(!m_private) return 0;
	return m_private->state.value(index);
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
	if(value(index) == m_private->puzzle->solution(index))
		return CorrectValue;
	if(m_private->puzzle->solution(index))
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
	if(value(index) == m_private->puzzle->solution(index))
		return CellInfo(CorrectValue, value(index));
	if(m_private->puzzle->solution(index))
		return CellInfo(WrongValue, value(index));
	return CellInfo(CorrectValue, value(index));
}

const QByteArray Game::allValues() const {
	if(!m_private) return QByteArray();
	
	return m_private->state.allValues();
}

QTime Game::time() const {
	if(!m_private) return QTime();
	
	return QTime().addMSecs(m_private->time.elapsed());
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

bool Game::canAddCheckpoint() const {
	return false;
}

bool Game::canUndo2Checkpoint() const {
	return false;
}

bool Game::userHadHelp() const {
	if(!m_private) return false;
	return m_private->hadHelp;
}

void Game::setUserHadHelp(bool hadHelp) {
	if(!m_private) return;
	m_private->hadHelp = hadHelp;
}

}

#include "ksudokugame.moc"
