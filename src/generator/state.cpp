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

#include "state.h"

#include <stdio.h>

State::State (QObject *             parent,
              const GuessesList &   guesses,
              int                   guessNumber,
              const BoardContents & values,
              const MoveList &      moves,
              const MoveList &      moveTypes)
    :
    QObject       (parent),
    m_guesses     (guesses),
    m_guessNumber (guessNumber),
    m_values      (values),
    m_moves       (moves),
    m_moveTypes   (moveTypes)
{
}

#include "state.moc"
