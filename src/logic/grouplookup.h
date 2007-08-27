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

#ifndef KSUDOKU_GROUPLOOKUP_H
#define KSUDOKU_GROUPLOOKUP_H

#include <cstdlib>

/**
 * This class provides an representation of a function mapping indexes to values within a
 * specific range.
 * All methods operate with O(c).
 * @note All methods don't perform a internal argument validation. You have to provide
 *       valid arguments or the results are undefined.
 */
typedef unsigned int uint;

class GroupLookup {
private:
	struct Value {
		int firstLookup;
		int lastLookup;
	};
	struct Lookup {
		uint value;
		int prevLookup;
		int nextLookup;
	};
	
public:
	GroupLookup(uint indices, uint values, uint defaultValue = 0) {
		m_valueCount = values;
		m_indexCount = indices;
		m_values = new Value[values];
		m_lookups = new Lookup[indices];
		
		for(uint i = 0; i < values; ++i) {
			m_values[i].firstLookup = -1;
			m_values[i].lastLookup = -1;
		}
		
		m_values[defaultValue].firstLookup = 0;
		m_values[defaultValue].lastLookup = indices-1;
		
		Lookup* lookup = &m_lookups[indices-1];
		lookup->value = defaultValue;
		lookup->prevLookup = indices-2;
		lookup->nextLookup = -1;
		--lookup;
		for(uint i = indices-2; i > 0; --i, --lookup) {
			lookup->value = defaultValue;
			lookup->prevLookup = i-1;
			lookup->nextLookup = i+1;
		}
		lookup->value = defaultValue;
		lookup->prevLookup = -1;
		lookup->nextLookup = 1;
	}
	GroupLookup(const GroupLookup& lookup) {
		m_valueCount = lookup.m_valueCount;
		m_indexCount = lookup.m_indexCount;
		m_values = new Value[m_valueCount];
		m_lookups = new Lookup[m_indexCount];
		memcpy(m_values, lookup.m_values, m_valueCount*sizeof(Value));
		memcpy(m_lookups, lookup.m_lookups, m_indexCount*sizeof(Lookup));
	}
	~GroupLookup() {
		delete[] m_values;
		delete[] m_lookups;
	}
	
	inline int firstIndexWithValue(uint value) {
		return m_values[value].firstLookup;
	}
	inline uint value(uint index) {
		return m_lookups[index].value;
	}
	inline void setValue(uint index, uint value) {
		Lookup* lookup = &m_lookups[index];
		
		if(lookup->value == value) return;
		
		if(lookup->prevLookup >= 0)
			m_lookups[lookup->prevLookup].nextLookup = lookup->nextLookup;
		else
			m_values[lookup->value].firstLookup = lookup->nextLookup;
		if(lookup->nextLookup >= 0)
			m_lookups[lookup->nextLookup].prevLookup = lookup->prevLookup;
		else
			m_values[lookup->value].lastLookup = lookup->prevLookup;
		
		Value* valueEntry = &m_values[value];
		lookup->value = value;
		if(valueEntry->lastLookup >= 0) {
			m_lookups[valueEntry->lastLookup].nextLookup = index;
			lookup->prevLookup = valueEntry->lastLookup;
			lookup->nextLookup = -1;
			valueEntry->lastLookup = index;
		} else {
			valueEntry->lastLookup = valueEntry->firstLookup = index;
			lookup->prevLookup = lookup->nextLookup = -1;
		}
	}
	
private:
	Value*  m_values;
	Lookup* m_lookups;
	uint    m_valueCount;
	uint    m_indexCount;
};

#endif // KSUDOKU_GROUPLOOKUP_H
