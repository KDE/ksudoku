/****************************************************************************
 *    Copyright 2011  Ian Wadham <iandw.au@gmail.com>                       *
 *                                                                          *
 *    This program is free software; you can redistribute it and/or         *
 *    modify it under the terms of the GNU General Public License as        *
 *    published by the Free Software Foundation; either version 2 of        *
 *    the License, or (at your option) any later version.                   *
 *                                                                          *
 *    This program is distributed in the hope that it will be useful,       *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *    GNU General Public License for more details.                          *
 *                                                                          *
 *    You should have received a copy of the GNU General Public License     *
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ****************************************************************************/

#ifndef STATE_H
#define STATE_H

#include <QObject>

#include "globals.h"
#include "sudokuboard.h"

class State : public QObject
{
    Q_OBJECT
public:
    State (QObject * parent,
           const GuessesList &   guesses,
           const int             guessNumber,
           const BoardContents & values,
           const MoveList &      moves,
           const MoveList &      moveTypes);

    GuessesList    guesses()              { return m_guesses; }
    int            guessNumber()          { return m_guessNumber; }
    BoardContents  values()               { return m_values;  }
    void           setGuessNumber (int n) { m_guessNumber = n; }
    MoveList       moves()                { return m_moves; }
    MoveList       moveTypes()            { return m_moveTypes; }

private:
    GuessesList    m_guesses;
    int            m_guessNumber;
    BoardContents  m_values;
    MoveList       m_moves;
    MoveList       m_moveTypes;
};

#endif // STATE_H
