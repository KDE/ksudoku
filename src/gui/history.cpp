#include "history.h"

namespace ksudoku {

HistoryEvent::HistoryEvent()
	: m_cellsIndex(), m_cellsBefore(), m_cellsAfter()
{
}

HistoryEvent::HistoryEvent(int index, const CellInfo& changeCell) 
	: m_cellsIndex(1, index), m_cellsBefore(), m_cellsAfter(1, changeCell)
{
}

HistoryEvent::HistoryEvent(const PuzzleState& puzzleChange)
	: m_cellsIndex(puzzleChange.size()), m_cellsBefore(), m_cellsAfter(puzzleChange.size())
{
	for(int i = 0; i < puzzleChange.size(); i++) {
		m_cellsIndex[i] = i;
		m_cellsAfter[i] = getPuzzleCell(puzzleChange, i);
	}
}

void HistoryEvent::setPuzzleCell(PuzzleState& puzzle, int index, const CellInfo& cell) const {
	switch(cell.state()) {
		case GivenValue:
			puzzle.setGiven(index, true);
			puzzle.resetMarkers(index);
			puzzle.setValue(index, cell.value());
			break;
		case ObviouslyWrong:
		case WrongValue:
		case CorrectValue:
			puzzle.setGiven(index, false);
			puzzle.resetMarkers(index);
			puzzle.setValue(index, cell.value());
			break;
		case Marker:
			puzzle.setGiven(index, false);
			puzzle.setValue(index, 0);
			puzzle.setMarkers(index, cell.markers());
			break;
	}
}

CellInfo HistoryEvent::getPuzzleCell(const PuzzleState& puzzle, int index) const {
	if(puzzle.given(index)) {
 		return CellInfo(GivenValue, puzzle.value(index));
	} else if(puzzle.value(index) == 0) {
		return CellInfo(puzzle.markers(index));
	} else {
		return CellInfo(CorrectValue, puzzle.value(index));
	}
}


bool HistoryEvent::applyTo(PuzzleState& puzzle) {
	if(m_cellsBefore.size() != 0 || m_cellsIndex.size() == 0)
		return false;
	
	m_cellsBefore = QVector<CellInfo>(m_cellsIndex.count());
	for(int i = 0; i < m_cellsIndex.count(); ++i) {
		m_cellsBefore[i] = getPuzzleCell(puzzle, m_cellsIndex[i]);
		setPuzzleCell(puzzle, m_cellsIndex[i], m_cellsAfter[i]);
	}
	return true;
}

bool HistoryEvent::undoOn(PuzzleState& puzzle) const {
	if(m_cellsBefore.size() == 0 || m_cellsBefore.size() != m_cellsIndex.size())
		return false;
	
	for(int i = 0; i < m_cellsIndex.count(); ++i) {
		setPuzzleCell(puzzle, m_cellsIndex[i], m_cellsBefore[i]);
	}
	return true;
}

bool HistoryEvent::redoOn(PuzzleState& puzzle) const {
	if(m_cellsBefore.size() == 0 || m_cellsBefore.size() != m_cellsIndex.size())
		return false;
	
	for(int i = 0; i < m_cellsIndex.count(); ++i) {
		setPuzzleCell(puzzle, m_cellsIndex[i], m_cellsAfter[i]);
	}
	return true;
}

}
