/***************************************************************************
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

#ifndef _KSUDOKUHISTORY_H_
#define _KSUDOKUHISTORY_H_

#include <qbitarray.h>
#include <QVector>
#include <qobject.h>
#include <qdatetime.h>

#include "ksudoku_types.h"


namespace ksudoku {
	
class CellInfo {
	public:
		inline CellInfo()
		  : m_state(Marker), m_value(0), m_markers()
		{ }
		inline CellInfo(ButtonState state, int value)
		  : m_state(state), m_value(value), m_markers()
		{ }
		inline CellInfo(const QBitArray& markers)
		  : m_state(Marker), m_value(0), m_markers(markers)
		{ }
		inline CellInfo(const CellInfo& info) 
		  : m_state(info.m_state), m_value(info.m_value), m_markers(info.m_markers)
		{ }
		inline ButtonState state() const { return m_state; }
		inline int value() const { return m_value; }
		inline const QBitArray markers() const { return m_markers; }
		inline bool marker(int value) const {
			if(value > m_markers.size() || value == 0) return false;
			return m_markers[value-1];
		}
		inline CellInfo& operator=(const CellInfo& info) {
			m_state = info.m_state;
			m_value = info.m_value;
			m_markers = info.m_markers;
			return *this;
		}
	private:
		ButtonState m_state;
		int m_value;
		QBitArray m_markers;
};

class PuzzleState {
public:
	PuzzleState() {
	}
	PuzzleState(int size, int values)
	  : m_markers(values), m_values(size, 0), m_given(size)
  
	{
		for(int i = 0; i < values; i++) {
			m_markers[i] = QBitArray(size);
		}
	}
public:
	void reset() {
		for(int i = 0; i < m_markers.size(); i++) {
			QBitArray &array = m_markers[i];
			for(int j = 0; j < array.size(); j++)
				array.clearBit(j);
		}
		for(int i = 0; i < m_values.size(); i++) {
			m_values[i] = 0;
			m_given.clearBit(i);
		}
	}
	inline void setMarker(int index, int value, bool status)
	{
		if(value == 0 || value > m_markers.size())
			return;
		m_markers[value-1].setBit(index, status);
	}
	inline void resetMarkers(int index)
	{
		for(int i = 0; i < m_markers.size(); i++) {
			m_markers[i].clearBit(index);
		}
	}
	inline void setMarkers(int index, const QBitArray& values) {
		if(values.size() == 0) {
			resetMarkers(index);
			return;
		}
		for(int i = 0; i < m_markers.size(); i++) {
			m_markers[i].setBit(index, values[i]);
		}
	}
	inline bool marker(int index, int value) const
	{
		if(value == 0 || value > m_markers.size())
			return false;
		return m_markers[value-1][index];
	}
	inline QBitArray markers(int index) const {
		QBitArray array(m_markers.size());
		for(int i = 0; i < m_markers.size(); i++) {
			array.setBit(i, m_markers[i][index]);
		}
		return array;
	}
	inline void setGiven(int index, bool given)
	{
		m_given.setBit(index, given);
	}
	inline void setValue(int index, int value)
	{
		m_values[index] = value;
	}
	inline void setValue(int index, int value, bool given)
	{
		m_given.setBit(index, given);
		m_values[index] = value;
	};
	inline bool given(int index) const
	{
		return m_given[index];
	}
	inline int value(int index) const
	{
		return m_values[index];
	}
// 	inline void operator=(const PuzzleState& state) {
// 		m_markers = state.m_markers;
// 		m_values = state.m_values;
// 		m_given = state.m_given;
// 	}
	inline void detach() {
		for(int i = 0; i < m_markers.size(); ++i) {
			// Detach m_markers
			// This actually is only needed once and not every loop
			// However this way it's more secure (m_markers.size() might be 0)
			m_markers[i] = m_markers[i];
			
			// Detach from shared bit array data
			m_markers[i].detach();
		}
		m_values.detach();
		m_given.detach();
	}
	inline const QByteArray allValues() const {
		return m_values;
	}
	/**
	 *@note Use this method only to get size of puzzle when operating
	 * directly on the puzzle state.
	 */
	inline int size() const {
		return m_values.size();
	}
	
private:
	QVector<QBitArray> m_markers;
	QByteArray m_values;
	QBitArray m_given;
};


class HistoryEvent {
	public:
		HistoryEvent();
		HistoryEvent(int index, const CellInfo& changedCell);
		HistoryEvent(const PuzzleState& newState);
		
		bool applyTo(PuzzleState& puzzle);
		bool undoOn(PuzzleState& puzzle) const;
		bool redoOn(PuzzleState& puzzle) const;
		
		const QVector<int>& cellIndices() const { return m_cellsIndex; }
		const QVector<CellInfo>& cellChanges() const { return m_cellsAfter; }
		
	private:
		void setPuzzleCell(PuzzleState& puzzle, int index, const CellInfo& cell) const;
		CellInfo getPuzzleCell(const PuzzleState& puzzle, int index) const;
		
	private:
		QVector<int> m_cellsIndex;
		QVector<CellInfo> m_cellsBefore;
		QVector<CellInfo> m_cellsAfter;
};

}

#endif
