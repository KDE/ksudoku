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

#ifndef GLOBALS_H
#define GLOBALS_H

#include <QVector>

enum SudokuType {Plain, XSudoku, Jigsaw, Samurai, TinySamurai, Roxdoku,
                 EndSudokuTypes};

enum Difficulty {VeryEasy  = 0, Easy = 1, Medium = 2, Hard = 3, Diabolical = 4,
                 Unlimited = 5};

enum Symmetry   {DIAGONAL_1, CENTRAL, LEFT_RIGHT, SPIRAL, FOURWAY,
                 RANDOM_SYM, LAST_CHOICE = RANDOM_SYM, NONE, DIAGONAL_2};

typedef QVector<int>      BoardContents;

typedef struct {
    char *     typeName;
    SudokuType type;
    int        blockSize;
    int        order;
    bool       generated;
    qint32     seed;
    int        nClues;
    int        nCells;
    int        nSingles;
    int        nSpots;
    int        nDeduces;
    int        nGuesses;
    int        firstGuessAt;
    float      rating;
    Difficulty difficulty;
}                         Statistics;

#endif // GLOBALS_H
