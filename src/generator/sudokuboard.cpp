/****************************************************************************
 *    Copyright 2011  Ian Wadham <iandw.au@gmail.com>                       *
 *    Copyright 2006  David Bau <david bau @ gmail com> Original algorithms *
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

#include "debug.h"

#include "sudokuboard.h"
#include "state.h"

#include <QMultiMap>
#include <QPrinter>
#include <QPainter>

#include <QTime>

#include <stdio.h>
#include <time.h>

SudokuBoard::SudokuBoard (QObject * parent,
                          SudokuType sudokuType, int blockSize)
    :
    QObject        (parent),
    m_type         (sudokuType),
    m_order        (blockSize * blockSize),
    m_blockSize    (blockSize),
    m_gridArea     (m_order * m_order),
    m_boardSize    (0),
    m_boardArea    (0),
    m_overlap      (0),
    m_fillStartRow (0),
    m_fillStartCol (0),
    m_nGroups      (0),
    m_groupSize    (0),
    m_vacant       (0),
    m_unusable     (-1)
{
    dbgLevel = 1;

    m_stats.type      = sudokuType;
    m_stats.blockSize = blockSize;
    m_stats.order     = m_order;
}

void SudokuBoard::setSeed()
{
    static bool started = false;
    if (started) {
        dbo "setSeed(): RESET IS TURNED OFF\n");
        // qsrand (m_stats.seed); // IDW test.
    }
    else {
        started = true;
        // m_stats.seed = 95985288; // IDW test.
        // m_stats.seed = 1317731144; // IDW test.
        m_stats.seed = time(0); // IDW test.
        qsrand (m_stats.seed);
        dbo "setSeed(): SEED = %d\n", m_stats.seed);
    }
}

void SudokuBoard::generatePuzzle (BoardContents & puzzle,
                                  BoardContents & solution,
                                  Difficulty difficultyRequired,
                                  Symmetry symmetry)
{
    QTime t;
    t.start();
    setSeed();
    int count = 0;

    if (symmetry == RANDOM_SYM) {	// Choose a symmetry at random.
        symmetry = (Symmetry) (qrand() % (int) LAST_CHOICE);
    }
    dbo "SYMMETRY IS %d\n", (int) symmetry);
    if (symmetry == DIAGONAL_1) {
	// If diagonal symmetry, choose between NW->SE and NE->SW diagonals.
        symmetry = (qrand() % 2 == 0) ? DIAGONAL_1 : DIAGONAL_2;
	dbo "Diagonal symmetry, choosing %s\n",
            (symmetry == DIAGONAL_1) ? "DIAGONAL_1" : "DIAGONAL_2");
    }

    while (true) {
        // Fill the board with values that satisfy the Sudoku rules but are
        // chosen in a random way: these values are the solution of the puzzle.
        solution = this->fillBoard();
        dbo "RETURN FROM fillBoard()\n");
        dbo "Time to fill board: %d msec\n", t.elapsed());

        // Randomly insert solution-values into an empty board until a point is
        // reached where all the cells in the solution can be logically deduced.
        puzzle = insertValues (solution, difficultyRequired, symmetry);
        dbo "RETURN FROM insertValues()\n");
        dbo "Time to do insertValues: %d msec\n", t.elapsed());

        if (difficultyRequired > m_stats.difficulty) {
            // Make the puzzle harder by removing values at random.
            puzzle = removeValues (solution, puzzle,
                                   difficultyRequired, symmetry);
            dbo "RETURN FROM removeValues()\n");
            dbo "Time to do removeValues: %d msec\n", t.elapsed());
        }

        Difficulty d = calculateRating (puzzle, 5);
        count++;
        dbo "CYCLE %d, achieved difficulty %d, required %d\n",
                         count, d, difficultyRequired);
        dbe "CYCLE %d, achieved difficulty %d, required %d\n",
                         count, d, difficultyRequired);

        if ((count >= 20) || (d >= difficultyRequired)) {
            // Exit after max attempts or when required difficulty is reached.
            break;
        }
    }

    if (dbgLevel > 0) {
        dbo "FINAL PUZZLE\n");
        print (puzzle);
        dbo "SOLUTION\n");
        print (solution);
    }

    // TODO - Description of puzzle - seed, date-time, type, blocksize, rating.

    fflush (stdout); // IDW test.
}

Difficulty SudokuBoard::calculateRating (const BoardContents & puzzle,
                                         int nSamples)
{
    float avGuesses;
    float avDeduces;
    float avDeduced;
    float fracClues;
    m_accum.nSingles = m_accum.nSpots = m_accum.nGuesses = m_accum.nDeduces = 0;
    m_accum.rating   = 0.0;

    BoardContents solution;
    clear (solution);
    setSeed();

    for (int n = 0; n < nSamples; n++) {
        dbo "SOLVE PUZZLE %d\n", n);
        solution = solveBoard (puzzle, nSamples == 1 ? NotRandom : Random);
        dbo "PUZZLE SOLVED %d\n", n);
        analyseMoves (m_stats);
        fracClues = float (m_stats.nClues) / float (m_stats.nCells);
        m_accum.nSingles += m_stats.nSingles;
        m_accum.nSpots   += m_stats.nSpots;
        m_accum.nGuesses += m_stats.nGuesses;
        m_accum.nDeduces += m_stats.nDeduces;
        m_accum.rating   += m_stats.rating;

        avDeduced = float(m_stats.nSingles + m_stats.nSpots) / m_stats.nDeduces;
        dbo2 "  Type %2d %2d: clues %3d %3d %2.1f%% %3d moves   %3dP %3dS %3dG "
             "%3dM %3dD %3.1fR\n",
             m_stats.type, m_stats.order,
             m_stats.nClues, m_stats.nCells,
             fracClues * 100.0, (m_stats.nCells -  m_stats.nClues),
             m_stats.nSingles, m_stats.nSpots, m_stats.nGuesses,
             (m_stats.nSingles + m_stats.nSpots + m_stats.nGuesses),
             m_stats.nDeduces, m_stats.rating);
    }

    avGuesses = float (m_accum.nGuesses) / nSamples;
    avDeduces = float (m_accum.nDeduces) / nSamples;
    avDeduced = float (m_accum.nSingles + m_accum.nSpots) / m_accum.nDeduces;
    m_accum.rating = m_accum.rating / nSamples;
    m_accum.difficulty = calculateDifficulty (m_accum.rating);
    dbo "  Av guesses %2.1f  Av deduces %2.1f"
        "  Av per deduce %3.1f  rating %2.1f difficulty %d\n",
        avGuesses, avDeduces, avDeduced, m_accum.rating, m_accum.difficulty);

    return m_accum.difficulty;
}

int SudokuBoard::checkPuzzle (const BoardContents & puzzle,
                              const BoardContents & solution)
{
    BoardContents answer = solveBoard (puzzle);
    if (answer.isEmpty()) {
        dbo1 "checkPuzzle: There is NO SOLUTION.\n");
        return -1;		// There is no solution.
    }
    if ((! solution.isEmpty()) && (answer != solution)) {
        dbo1 "checkPuzzle: The SOLUTION DIFFERS from the one supplied.\n");
        return -2;		// The solution differs from the one supplied.
    }

    analyseMoves (m_stats);

    answer.clear();
    answer = tryGuesses (Random);
    if (! answer.isEmpty()) {
        dbo1 "checkPuzzle: There is MORE THAN ONE SOLUTION.\n");
        return -3;		// There is more than one solution.
    }

    return calculateDifficulty (m_stats.rating);
}

BoardContents & SudokuBoard::solveBoard (const BoardContents & boardValues,
                                               GuessingMode gMode)
{
    if (dbgLevel >= 2) {
        dbo "solveBoard()\n");
        print (boardValues);
    }
    m_currentValues = boardValues;
    return solve (gMode);
}

BoardContents & SudokuBoard::solve (GuessingMode gMode = Random)
{
    // Eliminate any previous solver work.
    qDeleteAll (m_states);
    m_states.clear();

    m_moves.clear();
    m_moveTypes.clear();
    int nClues = 0;
    int nCells = 0;
    int value  = 0;
    for (int n = 0; n < m_boardArea; n++) {
        value = m_currentValues.at(n);
        if (value != m_unusable) {
            nCells++;
            if (value != m_vacant) {
                nClues++;
            }
        }
    }
    m_stats.nClues = nClues;
    m_stats.nCells = nCells;
    dbo1 "STATS: CLUES %d, CELLS %d, PERCENT %.1f\n", nClues, nCells,
                                        nClues * 100.0 / float (nCells));

    // Attempt to deduce the solution in one hit.
    GuessesList g = deduceValues (m_currentValues, gMode);
    if (g.isEmpty()) {
        // The entire solution can be deduced by applying the Sudoku rules.
        dbo1 "NO GUESSES NEEDED, the solution can be entirely deduced.\n");
        return m_currentValues;
    }

    // We need to use a mix of guessing, deducing and backtracking.
    m_states.push (new State (this, g, 0,
                   m_currentValues, m_moves, m_moveTypes));
    return tryGuesses (gMode);
}

BoardContents & SudokuBoard::tryGuesses (GuessingMode gMode = Random)
{
    while (m_states.count() > 0) {
        GuessesList guesses = m_states.top()->guesses();
        int n = m_states.top()->guessNumber();
        if ((n >= guesses.count()) || (guesses.at (0) == -1)) {
            dbo2 "POP: Out of guesses at level %d\n", m_states.count());
            delete m_states.pop();
            if (m_states.count() > 0) {
                m_moves.clear();
                m_moveTypes.clear();
                m_moves = m_states.top()->moves();
                m_moveTypes = m_states.top()->moveTypes();
            }
            continue;
        }
        m_states.top()->setGuessNumber (n + 1);
        m_currentValues = m_states.top()->values();
        m_moves.append (guesses.at(n));
        m_moveTypes.append (Guess);
        m_currentValues [pairPos (guesses.at(n))] = pairVal (guesses.at(n));
        dbo2 "\nNEXT GUESS: level %d, guess number %d\n",
                m_states.count(), n);
        dbo2 "  Pick %d %d row %d col %d\n",
                pairVal (guesses.at(n)), pairPos (guesses.at(n)),
                pairPos (guesses.at(n))/m_boardSize + 1,
                pairPos (guesses.at(n))%m_boardSize + 1);

        guesses = deduceValues (m_currentValues, gMode);

        if (guesses.isEmpty()) {
            // TODO: IDW - Delete the stack somewhere else.  It is needed by
            //             checkPuzzle() for the multiple-solutions test.
            // while (! m_states.isEmpty()) {
                // dbo2 "POP: No more guesses needed at level %d\n",
                        // m_states.count());
                // delete m_states.pop();
            // }
            return m_currentValues;
        }
        m_states.push (new State (this, guesses, 0,
                       m_currentValues, m_moves, m_moveTypes));
    }

    // No solution.
    m_currentValues.clear();
    return m_currentValues;
}

BoardContents SudokuBoard::insertValues (const BoardContents & solution,
                                         const Difficulty      required,
                                         const Symmetry        symmetry)
{
    BoardContents puzzle;
    BoardContents filled;
    QVector<int> sequence (m_boardArea);
    int cell  = 0;
    int value = 0;

    // Set up empty board areas.
    clear (puzzle);
    clear (filled);

    // Add cells in random order, but skip cells that can be deduced from them.
    dbo1 "Start INSERTING: %d solution values\n", solution.count());
    randomSequence (sequence);

    int index = 0;
    for (int n = 0; n < m_boardArea; n++) {
        cell  = sequence.at (n);
        value = filled.at (cell);
        if (filled.at (cell) == 0) {
            index = n;
            changeClues (puzzle, cell, symmetry, solution);
            changeClues (filled, cell, symmetry, solution);

            dbo2 "CALL deduceValues():\n");
            deduceValues (filled, Random /* NotRandom */);
            dbo2 "BACK FROM deduceValues():\n");
            if (dbgLevel >= 3) {
                print (puzzle);
                print (filled);
            }
        }
    }
    print (puzzle); // IDW test.

    while (true) {
        // Check the difficulty of the puzzle.
        solveBoard (puzzle);
        analyseMoves (m_stats);
        m_stats.difficulty = calculateDifficulty (m_stats.rating);
        if (m_stats.difficulty <= required) {
            break;	// The difficulty is as required or not enough yet.
        }
        // The puzzle needs to be made easier.  Add randomly-selected clues.
        for (int n = index; n < m_boardArea; n++) {
            cell  = sequence.at (n);
            if (puzzle.at (cell) == 0) {
                changeClues (puzzle, cell, symmetry, solution);
                index = n;
                break;
            }
        }
        dbo1 "At index %d, added value %d, cell %d, row %d, col %d\n",
                index, solution.at (cell),
                cell, cell/m_boardSize + 1, cell%m_boardSize + 1);
    }
    print (puzzle);
    return puzzle;
}

BoardContents SudokuBoard::removeValues (const BoardContents & solution,
                                               BoardContents & puzzle,
                                         const Difficulty      required,
                                         const Symmetry        symmetry)
{
    // Make the puzzle harder by removing values at random, making sure at each
    // step that the puzzle has a solution, the correct solution and only one
    // solution.  Stop when these conditions can no longer be met and the
    // required difficulty is reached or failed to be reached with the current
    // (random) selection of board values.

    // Remove values in random order, but put them back if the solution fails.
    BoardContents vacant;
    QVector<int> sequence (m_boardArea);
    int          cell       = 0;
    int          value      = 0;
    QList<int>   tailOfRemoved;

    // No guesses until this much of the puzzle, including clues, is filled in.
    float        guessLimit = 0.6;
    int          noGuesses  = (int) (guessLimit * m_stats.nCells + 0.5);
    dbo1 "Guess limit = %.2f, nCells = %d, nClues = %d, noGuesses = %d\n",
            guessLimit, m_stats.nCells, m_stats.nClues, noGuesses);

    dbo1 "Start REMOVING:\n");
    randomSequence (sequence);
    clear (vacant);

    for (int n = 0; n < m_boardArea; n++) {
        cell  = sequence.at (n);
        value = puzzle.at (cell);
        if ((value == 0) || (value == m_unusable)) {
            continue;			// Skip empty or unusable cells.
        }
	// Try removing this clue and its symmetry partners (if any).
	changeClues (puzzle, cell, symmetry, vacant);
        dbo1 "ITERATION %d: Removed %d from cell %d\n", n, value, cell);

        // Check the solution is still OK and calculate the difficulty roughly.
        int result = checkPuzzle (puzzle, solution);

        // Do not force the human solver to start guessing too soon.
        if ((result >= 0) && (required != Unlimited) &&
            (m_stats.firstGuessAt <= (noGuesses - m_stats.nClues))) {
            dbo1 "removeValues: FIRST GUESS is too soon: move %d of %d.\n",
                    m_stats.firstGuessAt, m_stats.nCells - m_stats.nClues);
            result = -4;
        }

        // If the solution is not OK, replace the removed value(s).
        if (result < 0) {
            dbo1 "ITERATION %d: Replaced %d at cell %d, check returned %d\n",
                    n, value, cell, result);
	    changeClues (puzzle, cell, symmetry, solution);
        }

        // If the solution is OK, check the difficulty (roughly).
        else {
            m_stats.difficulty = (Difficulty) result;
            dbo1 "CURRENT DIFFICULTY %d\n", m_stats.difficulty);

            if (m_stats.difficulty == required) {
                // Save removed positions while the difficulty is as required.
                tailOfRemoved.append (cell);
                dbo1 "OVERSHOOT %d at sequence %d\n",
                        tailOfRemoved.count(), n);
            }

            else if (m_stats.difficulty > required) {
                // Finish if the required difficulty is exceeded.
                dbo1 "BREAK on difficulty %d\n", m_stats.difficulty);
                dbe1 "BREAK on difficulty %d\n", m_stats.difficulty);
                dbo1 "Replaced %d at cell %d, overshoot is %d\n",
                        value, cell, tailOfRemoved.count());
                // Replace the value involved.
	        changeClues (puzzle, cell, symmetry, solution);
                break;
            }
        }
    }

    // If the required difficulty was reached and was not Unlimited, replace
    // half the saved values.
    //
    // This should avoid chance fluctuations in the calculated difficulty (when
    // the solution involves guessing) and provide a puzzle that is within the
    // required difficulty range.
    if ((required != Unlimited) && (tailOfRemoved.count() > 1)) {
        for (int k = 0; k < tailOfRemoved.count() / 2; k++) {
            cell = tailOfRemoved.takeLast();
            dbo1 "Replaced clue(s) for cell %d\n", cell);
            changeClues (puzzle, cell, symmetry, solution);
        }
    }
    return puzzle;
}

void SudokuBoard::analyseMoves (Statistics & s)
{
    dbo1 "\nanalyseMoves()\n");
    s.nCells       = m_stats.nCells;
    s.nClues       = m_stats.nClues;
    s.firstGuessAt = s.nCells - s.nClues + 1;

    s.nSingles = s.nSpots = s.nDeduces = s.nGuesses = 0;
    Move m;
    Move mType;
    while (! m_moves.isEmpty()) {
        m = m_moves.takeFirst();
        mType = m_moveTypes.takeFirst();
        switch (mType) {
        case Single:
            dbo2 "  Single Pick %d %d row %d col %d\n",
                    pairVal(m), pairPos(m), pairPos(m)/m_boardSize + 1,
                                            pairPos(m)%m_boardSize + 1);
            s.nSingles++;
            break;
        case Spot:
            dbo2 "  Single Spot %d %d row %d col %d\n",
                    pairVal(m), pairPos(m), pairPos(m)/m_boardSize + 1,
                                            pairPos(m)%m_boardSize + 1);
            s.nSpots++;
            break;
        case Deduce:
            dbo2 "Deduce: Iteration %d\n", m);
            s.nDeduces++;
            break;
        case Guess:
            dbo2 "GUESS:        %d %d row %d col %d\n",
                    pairVal(m), pairPos(m), pairPos(m)/m_boardSize + 1,
                                            pairPos(m)%m_boardSize + 1);
            if (s.nGuesses < 1) {
                s.firstGuessAt = s.nSingles + s.nSpots + 1;
            }
            s.nGuesses++;
            break;
        case Wrong:
            dbo2 "WRONG GUESS:  %d %d row %d col %d\n",
                    pairVal(m), pairPos(m), pairPos(m)/m_boardSize + 1,
                                            pairPos(m)%m_boardSize + 1);
            break;
        case Result:
            break;
        }
    }

    // Calculate the empirical formula for the difficulty rating.  Note that
    // guess-points are effectively weighted by 3, because the deducer must
    // always iterate one more time to establish that a guess is needed.
    s.rating = 2 * s.nGuesses + s.nDeduces - (float(s.nClues)/s.nCells);

    // Calculate the difficulty level for empirical ranges of the rating.
    s.difficulty = calculateDifficulty (s.rating);

    dbo1  // IDW test.
         "  aM: Type %2d %2d: clues %3d %3d %2.1f%%   %3dP %3dS %3dG "
         "%3dM %3dD %3.1fR D=%d\n\n",
         m_stats.type, m_stats.order,
         s.nClues, s.nCells, ((float) s.nClues / s.nCells) * 100.0,
         s.nSingles, s.nSpots, s.nGuesses, (s.nSingles + s.nSpots + s.nGuesses),
         s.nDeduces, s.rating, s.difficulty);
}

Difficulty SudokuBoard::calculateDifficulty (float rating)
{
    // These ranges of the rating were arrived at empirically by solving a few
    // dozen published puzzles and comparing SudokuBoard's rating value with the
    // description of difficulty given by the publisher, e.g. Diabolical or Evil
    // puzzles gave ratings in the range 10.0 to 20.0, so became Diabolical.

    Difficulty d = Unlimited;

    if (rating < 1.7) {
        d = VeryEasy;
    }
    else if (rating < 2.7) {
        d = Easy;
    }
    else if (rating < 4.6) {
        d = Medium;
    }
    else if (rating < 10.0) {
        d = Hard;
    }
    else if (rating < 20.0) {
        d = Diabolical;
    }

    return d;
}

void SudokuBoard::print (const BoardContents & boardValues)
{
    // Used for test and debug, but the format is also parsable and loadable.

    char nLabels[] = "123456789";
    char aLabels[] = "abcdefghijklmnopqrstuvwxy";
    int index, value;

    if (boardValues.size() != m_boardArea) {
        printf ("Error: %d board values to be printed, %d values required.\n\n",
            boardValues.size(), m_boardArea);
        return;
    }
    for (int i = 0; i < m_boardSize; i++) {
        if ((i != 0) && (i % m_blockSize == 0)) {
            printf ("\n");		// Gap between square blocks.
        }
        for (int j = 0; j < m_boardSize; j++) {
            index = i * m_boardSize + j;
            value = boardValues.at (index);
            if (j % m_blockSize == 0) {
                printf ("  ");		// Gap between square blocks.
            }
            if (value == m_unusable) {
                printf (" '");		// Unused cell (e.g. in Samurai).
            }
            else if (value == 0) {
                printf (" -");		// Empty cell (to be solved).
            }
            else {
                value--;
                char label = (m_order > 9) ? aLabels[value] : nLabels[value];
                printf (" %c", label);	// Given cell (or clue).
            }
        }
        printf ("\n");			// End of row.
    }
    printf ("\n");			// End of puzzle/solution.
}

GuessesList SudokuBoard::deduceValues (BoardContents & boardValues,
                                       GuessingMode gMode = Random)
{
    int iteration = 0;
    setUpValueRequirements (boardValues);
    while (true) {
        iteration++;
        m_moves.append (iteration);
        m_moveTypes.append (Deduce);
        dbo2 "DEDUCE: Iteration %d\n", iteration);
        bool stuck = true;
        int  count = 0;
        GuessesList guesses;

        for (int cell = 0; cell < m_boardArea; cell++) {
            if (boardValues.at (cell) == m_vacant) {
                GuessesList newGuesses;
                qint32 numbers = m_validCellValues.at (cell);
                // dbo3 ("Cell %d, valid numbers %03o\n", cell, numbers);
                if (numbers == 0) {
                    dbo2 "SOLUTION FAILED: RETURN at cell %d\n", cell);
                    return solutionFailed (guesses);
                }
                int validNumber = 1;
                while (numbers != 0) {
                    // dbo3 "Numbers = %03o, validNumber = %d\n",
                    //         numbers, validNumber);
                    if (numbers & 1) {
                        newGuesses.append (setPair (cell, validNumber));
                    }
                    numbers = numbers >> 1;
                    validNumber++;
                }
                if (newGuesses.count() == 1) {
                    m_moves.append (newGuesses.first());
                    m_moveTypes.append (Single);
                    boardValues [cell] = pairVal (newGuesses.takeFirst());
                    dbo3 "  Single Pick %d %d row %d col %d\n",
                            boardValues.at (cell), cell,
                            cell/m_boardSize + 1, cell%m_boardSize + 1);
                    updateValueRequirements (boardValues, cell);
                    stuck = false;
                }
                else if (stuck) {
                    // Select a list of guesses.
                    if (guesses.isEmpty() ||
                        (newGuesses.count() < guesses.count())) {
                        guesses = newGuesses;
                        count = 1;
                    }
                    else if (newGuesses.count() > guesses.count()) {
                        ;
                    }
                    else if (gMode == Random) {
                        if ((qrand() % count) == 0) {
                            guesses = newGuesses;
                        }
                        count++;
                    }
                }
            } // End if
        } // Next cell

        for (int group = 0; group < m_nGroups; group++) {
            qint32 numbers = m_requiredGroupValues.at (group);
            // dbo3 "Group %d, valid numbers %03o\n", group, numbers);
            if (numbers == 0) {
                continue;
            }
            int    validNumber = 1;
            qint32 bit         = 1;
            int    cell        = 0;
            while (numbers != 0) {
                if (numbers & 1) {
                    GuessesList newGuesses;
                    int index = group * m_groupSize;
                    for (int n = 0; n < m_groupSize; n++) {
                        cell = m_groupList.at (index);
                        if ((m_validCellValues.at (cell) & bit) != 0) {
                            newGuesses.append (setPair (cell, validNumber));
                        }
                        index++;
                    }
                    if (newGuesses.isEmpty()) {
                        dbo2 "SOLUTION FAILED: RETURN at group %d\n", group);
                        return solutionFailed (guesses);
                    }
                    else if (newGuesses.count() == 1) {
                        m_moves.append (newGuesses.first());
                        m_moveTypes.append (Spot);
                        cell = pairPos (newGuesses.takeFirst());
                        boardValues [cell] = validNumber;
                        dbo3 "  Single Spot in Group %d value %d %d "
                                "row %d col %d\n",
                                group, validNumber, cell,
                                cell/m_boardSize + 1, cell%m_boardSize + 1);
                        updateValueRequirements (boardValues, cell);
                        stuck = false;
                    }
                    else if (stuck) {
                        // Select a list of guesses.
                        if (guesses.isEmpty() ||
                            (newGuesses.count() < guesses.count())) {
                            guesses = newGuesses;
                            count = 1;
                        }
                        else if (newGuesses.count() > guesses.count()) {
                            ;
                        }
                        else if (gMode == Random){
                            if ((qrand() % count) == 0) {
                                guesses = newGuesses;
                            }
                            count++;
                        }
                    }
                } // End if (numbers & 1)
                numbers = numbers >> 1;
                bit     = bit << 1;
                validNumber++;
            } // Next number
        } // Next group

        if (stuck) {
            GuessesList original = guesses;
            if (gMode == Random) {
                // Shuffle the guesses.
                QVector<int> sequence (guesses.count());
                randomSequence (sequence);

                guesses.clear();
                for (int i = 0; i < original.count(); i++) {
                    guesses.append (original.at (sequence.at (i))); 
                }
            }
            dbo2 "Guess    ");
            for (int i = 0; i < original.count(); i++) {
                dbo3 "%d,%d ",
                        pairPos (original.at(i)), pairVal (original.at(i)));
            }
            dbo2 "\n");
            dbo2 "Shuffled ");
            for (int i = 0; i < guesses.count(); i++) {
                dbo3 "%d,%d ",
                        pairPos (guesses.at (i)), pairVal (guesses.at(i)));
            }
            dbo2 "\n");
            return guesses;
        }
    } // End while (true)
}

GuessesList SudokuBoard::solutionFailed (GuessesList & guesses)
{
    guesses.clear();
    guesses.append (-1);
    return guesses;
}

void SudokuBoard::randomSequence (QVector<int> & sequence)
{
    if (sequence.isEmpty()) return;

    // Fill the vector with consecutive integers.
    int size = sequence.size();
    for (int i = 0; i < size; i++) {
        sequence [i] = i;
    }

    if (size == 1) return;

    // Shuffle the integers.
    int last = size;
    int z    = 0;
    int temp = 0;
    for (int i = 0; i < size; i++) {
        z = qrand() % last;
        last--;
        temp            = sequence.at (z);
        sequence [z]    = sequence.at (last);
        sequence [last] = temp;
    }
}

void SudokuBoard::setUpValueRequirements (BoardContents & boardValues)
{
    // Set a 1-bit for each possible cell-value in this order of Sudoku, for
    // example 9 bits for a 9x9 grid with 3x3 blocks.
    qint32 allValues = (1 << m_order) - 1;

    if (dbgLevel >= 2) {
        this->print (boardValues);
    }

    // Set bit-patterns to show what values each row, col or block needs.
    // The starting pattern is allValues, but bits are set to zero as the
    // corresponding values are supplied during puzzle generation and solving.

    m_requiredGroupValues.fill (0, m_nGroups);
    int    index = 0;
    qint32 bitPattern = 0;
    for (int group = 0; group < m_nGroups; group++) {
        bitPattern = 0;
        for (int n = 0; n < m_groupSize; n++) {
            int value = boardValues.at (m_groupList.at (index)) - 1;
            if (value >= 0) {
                bitPattern |= (1 << value);	// Add bit for each value found.
            }
            index++;
        }
        // Reverse all the bits, giving values currently not found in the group.
        m_requiredGroupValues [group] = bitPattern ^ allValues;
    }

    // Set bit-patterns to show that each cell can accept any value.  Bits are
    // set to zero as possibilities for each cell are eliminated when solving.
    m_validCellValues.fill (allValues, m_boardArea);
    for (int i = 0; i < m_boardArea; i++) {
        if (boardValues.at (i) == m_unusable) {
            // No values are allowed in unusable cells (e.g. in Samurai type).
            m_validCellValues [i] = 0;
        }
        if (boardValues.at (i) != m_vacant) {
            // Cell is already filled in.
            m_validCellValues [i] = 0;
        }
    }

    // Now, for each cell, retain bits for values that are required by every
    // group to which that cell belongs.  For example, if the row already has 1,
    // 2, 3, the column has 3, 4, 5, 6 and the block has 6, 9, then the cell
    // can only have 7 or 8, with bit value 192.
    index = 0;
    for (int group = 0; group < m_nGroups; group++) {
        for (int n = 0; n < m_order; n++) {
            int cell  = m_groupList.at (index);
            m_validCellValues [cell] &= m_requiredGroupValues.at (group);
            index++;
        }   
    }
    dbo2 "Finished setUpValueRequirements()\n");

    dbo3 "allowed:\n");
    for (int i = 0; i < m_boardArea; i++) {
         dbo3 "'%03o', ", m_validCellValues.at (i));
        if ((i + 1) % m_boardSize == 0) dbo3 "\n");
    }
    dbo3 "needed:\n");
    for (int group = 0; group < m_nGroups; group++) {
        dbo3 "'%03o', ", m_requiredGroupValues.at (group));
        if ((group + 1) % m_order == 0) dbo3 "\n");
    }
    dbo3 "\n");
}

void SudokuBoard::updateValueRequirements (BoardContents & boardValues, int cell)
{
    // Set a 1-bit for each possible cell-value in this order of Sudoku.
    qint32 allValues  = (1 << m_order) - 1;
    // Set a complement-mask for this cell's new value.
    qint32 bitPattern = (1 << (boardValues.at (cell) - 1)) ^ allValues;
    // Show that this cell no longer requires values: it has been filled.
    m_validCellValues [cell] = 0;

    // Update the requirements for each group to which this cell belongs.
    int index = m_cellIndex.at (cell);
    int inmax = m_cellIndex.at (cell + 1);
    for (int i = index; i < inmax; i++) {
        int group  = m_cellGroups.at (i);
        m_requiredGroupValues [group] &= bitPattern;

        int offset = group * m_groupSize;
        for (int n = 0; n < m_order; n++) {
            int cell  = m_groupList.at (offset + n);
            m_validCellValues [cell] &= bitPattern;
        }   
    }
}

qint32 SudokuBoard::indexSquareBlock (qint32 index, int topLeft)
{
    int cell      = topLeft;
    qint32 offset = index;
    for (int i = 0; i < m_blockSize; i++) {
        for (int j = 0; j < m_blockSize; j++) {
            m_groupList [offset] = cell;
            // dbo3 "Index block [%d,%d], "
                    // "value %d at offset %d.\n",
                    // i, j, cell, offset);
            cell++;
            offset++;
        }
        cell = cell + m_boardSize - m_blockSize;
    }

    return offset;
}

void SudokuBoard::indexCellsToGroups()
{
    QMultiMap<int, int> cellsToGroups;
    for (int g = 0; g < m_nGroups; g++) {
        int offset = g * m_groupSize;
        for (int n = 0; n < m_groupSize; n++) {
            cellsToGroups.insert (m_groupList.at (offset + n), g);
        }
    }

    m_cellIndex.fill  (0, m_boardArea + 1);
    m_cellGroups.fill (0, m_nGroups * m_groupSize);
    int index = 0;
    for (int cell = 0; cell < m_boardArea; cell++) {
        m_cellIndex [cell] = index;
        QList<int> groups = cellsToGroups.values (cell);
        foreach (int g, groups) {
            m_cellGroups [index] = g;
            index++;
        }
    }
    m_cellIndex [m_boardArea] = index;

    dbo3 "indexCellsToGroups():\n");
    for (int cell = 0; cell < m_boardArea; cell++) {
        // dbo3 "Cell %3d: groups: ", cell);
        int index = m_cellIndex.at (cell);
        int inmax = m_cellIndex.at (cell + 1);
        for (int n = index; n < inmax; n++) {
            // dbo3 " %3d", m_cellGroups.at (n));
        }
        // dbo3 "\n");
    }
}

void SudokuBoard::markUnusable (BoardContents & boardValues,
                            int imin, int imax, int jmin, int jmax)
{
    dbo3 "markUnusable(): Row %2d to %2d, col %2d to %2d\n", // IDW test.
            imin + 1, imax, jmin + 1, jmax); // IDW test.
    int index = 0;
    for (int i = imin; i < imax; i++) {
        for (int j = jmin; j < jmax; j++) {
            index = i * m_boardSize + j;
            boardValues [index] = m_unusable;
        }
    }
}

void SudokuBoard::changeClues (BoardContents & to, int cell, Symmetry type,
                               const BoardContents & from)
{
    int nSymm = 1;
    int indices[4];
    nSymm = getSymmetricIndices (m_boardSize, type, cell, indices);
    for (int k = 0; k < nSymm; k++) {
        cell = indices [k];
        to [cell] = from.at (cell);
    }
}

int SudokuBoard::getSymmetricIndices
                (int size, Symmetry type, int index, int * out)
{
    int result = 1;
    int row    = index / size;
    int col    = index % size;
    out[0]     = index;
    bool b[3]  = {1, 1, 1};

    switch (type) {
        case NONE:
            break;
        case DIAGONAL_1:
	    // Reflect a copy of the point around two central axes making its
	    // reflection in the NW-SE diagonal the same as for NE-SW diagonal.
            row = size - row - 1;
            col = size - col - 1;
            // No break; fall through to case DIAGONAL_2.
        case DIAGONAL_2:
            out[1] = (col * size) + row;	// Reflect in NW-SE diagonal.
            result = (out[1] == out[0]) ? 1 : 2;
            break;
        case CENTRAL:
            out[1] = (size * size) - index - 1;
            result = (out[1] == out[0]) ? 1 : 2;
            break;
        case FOURWAY:
	    /* This could be WHEEL, SWIRL, WHIRLPOOL or SPIRAL symmetry? */
	    /*
	    result = 4;
            if(size % 2 == 1) {
		if ((row == col) && (col == (size - 1)/2)) {	// Central cell.
		    result = 1;
		}
	    }
	    if (result == 4) {
                out[1] = (size - row - 1) * size + size - col - 1;
                out[2] = col * size + size - row - 1;
                out[3] = (size - col - 1) * size + row;
	    }
	    */
            out[1] = out[2] = out[3] = 0;
            if(size % 2 == 1) {
                if(col == (size - 1)/2) {
                    b[0] = b[2] = 0;
                }
                if(row == (size - 1)/2) {
                    b[1] = b[2] = 0;
                }
            }

            if(b[2] == 0) {
                out[1] = (size - row - 1) * size + size - col - 1;
                if(out[1] != out[0]) {
		    result++;
		}
            }
            else {
                out[1] = (size - row - 1) * size + (size - col - 1);
                out[2] = row * size + (size - col - 1);
                out[3] = (size - row - 1) * size + col;
                result = 4;
            }
            break;
        case LEFT_RIGHT:
            out[1] = (size - 1 - row) * size + col;
            result = (out[1] == out[0]) ? 1 : 2;
            break;
        case TOP_BOTTOM:
	    out[1] = (row) * size + size - col - 1;
            result = (out[1] == out[0]) ? 1 : 2;
            break;
        default:
            break;
    }
    return result;
}

void SudokuBoard::sendToPrinter (const BoardContents & boardValues)
{
    QString labels = (m_blockSize <= 3) ? "123456789" :
                                          "ABCDEFGHIJKLMNOPQRSTUVWXY";

    // This gets us to the installation's default printer (i.e. no dialog).
    QPrinter printer (QPrinter::HighResolution);
    int pixels = qMin (printer.width(), printer.height());
    dbo "Printer has w = %d, h = %d, pix = %d\n",
            printer.width(), printer.height(), pixels);

    int size    = pixels - (pixels / 20);	// Allow about 2.5% each side.
    int divs    = (m_boardSize > 18) ? m_boardSize : 18;
    int sCell   = size / divs;
    size        = m_boardSize * sCell;
    int margin1 = (pixels - size) / 2;
    int margin2 = (qMax (printer.width(), printer.height()) - size) / 2;
    int topX    = (printer.width() < printer.height()) ? margin1 : margin2;
    int topY    = (printer.width() < printer.height()) ? margin2 : margin1;
    dbo "margin1 %d, margin2 %d, size %d, topX %d, topY %d\n",
            margin1, margin2, size, topX, topY);

    int thin    = sCell / 40;		// Allow 0.25%.
    int thick   = (thin > 0) ? 2 * thin : 2;
    int nLines  = m_order + 1;
    dbo "thin %d, thick = %d, nLines = %d\n", thin, thick, nLines);

    QPen light (QColor(QString("lightblue")));
    QPen heavy (QColor(QString("black")));
    light.setWidth (thin);
    heavy.setWidth (thick);

    QPainter p (&printer);
    QFont    f = p.font();

    f.setPixelSize ((sCell * 6) / 10);		// Font size 60% height of cell.
    p.setFont (f);

    // Draw one or more grids (five overlapping grids in Samurai layout).
    int length = m_order * sCell;
    int pos    = 0;
    int gridX  = 0;
    int gridY  = 0;
    for (int g = 0; g < ((m_type == Samurai) ? 5 : 1); g++) {
        switch (g) {
        case 0:			// Main grid or top-left grid in Samurai layout.
            gridX = topX;
            gridY = topY;
            break;
        case 1:			// Top-right grid in Samurai layout.
            gridX = topX + (2 * m_order - 2 * m_overlap) * sCell;
            gridY = topY;
            break;
        case 2:			// Centre grid in Samurai layout.
            gridX = topX + (m_order - m_overlap) * sCell;
            gridY = topY + (m_order - m_overlap) * sCell;
            break;
        case 3:			// Bottom-left grid in Samurai layout.
            gridX = topX;
            gridY = topY + (2 * m_order - 2 * m_overlap) * sCell;
            break;
        case 4:			// Bottom-right grid in Samurai layout.
            gridX = topX + (2 * m_order - 2 * m_overlap) * sCell;
            gridY = topY + (2 * m_order - 2 * m_overlap) * sCell;
            break;
        }

        // Draw the faint lines that go inside the blocks.
        p.setPen (light);
        for (int n = 0; n < nLines; n++) {
            if ((n % 3) == 0) continue;
            pos = n * sCell;
            p.drawLine (gridX + pos, gridY, gridX + pos, gridY + length);
            p.drawLine (gridX, gridY + pos, gridX + length, gridY + pos);
        }

        // Draw the heavy lines that surround the blocks.
        p.setPen (heavy);
        for (int n = 0; n < nLines; n++) {
            if ((n % 3) > 0) continue;
            pos = n * sCell;
            p.drawLine (gridX + pos, gridY, gridX + pos, gridY + length);
            p.drawLine (gridX, gridY + pos, gridX + length, gridY + pos);
        }
    }

    // Fill in the cell contents.
    for (int n = 0; n < m_boardArea; n++) {
        int row = n / m_boardSize;
        int col = n % m_boardSize;
        QRectF rect (topX + sCell * col, topY + sCell * row, sCell, sCell);
        if (boardValues.at (n) > 0) {
            p.drawText (rect, Qt::AlignCenter,
                        labels.mid (boardValues.at(n) - 1, 1));
        }
    }
}

#include "sudokuboard.moc"