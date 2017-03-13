/****************************************************************************
 *    Copyright 2011  Ian Wadham <iandw.au@gmail.com>                       *
 *    Copyright 2006  David Bau <david bau @ gmail com> Original algorithms *
 *    Copyright 2015  Ian Wadham <iandw.au@gmail.com>                       *
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
#include "ksudoku_logging.h"

#include "sudokuboard.h"
#include "state.h"
#include "mathdokugenerator.h"
#include <QDebug>

#include <KLocalizedString>
#include <KMessageBox>

#include <QMultiMap>

#include <QTime>

#include <stdio.h>
#include <ctime>

SudokuBoard::SudokuBoard (SKGraph * graph)
    :
    m_type         (graph->specificType()),
    m_order        (graph->order()),
    m_blockSize    (graph->base()),
    m_boardSize    (0),
    m_boardArea    (graph->size()),
    m_overlap      (0),
    m_nGroups      (graph->cliqueCount()),
    m_groupSize    (m_order),
    m_graph        (graph),
    m_vacant       (VACANT),
    m_unusable     (UNUSABLE)
{
    m_stats.type      = m_type;
    m_stats.blockSize = m_blockSize;
    m_stats.order     = m_order;
    m_boardSize       = graph->sizeX(); // TODO - IDW. Rationalise grid sizes.
    qCDebug(KSudokuLog) << "SudokuBoard: type " << m_type << graph->name()
                        << ", block " << m_blockSize << ", order " << m_order
                        << ", BoardArea " << m_boardArea;
}

void SudokuBoard::setSeed()
{
    static bool started = false;
    if (started) {
        qCDebug(KSudokuLog) << "setSeed(): RESET IS TURNED OFF";
        // qsrand (m_stats.seed); // IDW test.
    }
    else {
        started = true;
        m_stats.seed = std::time(nullptr);
        qsrand (m_stats.seed);
        qCDebug(KSudokuLog) << "setSeed(): SEED = " << m_stats.seed;
    }
}

bool SudokuBoard::generatePuzzle             (BoardContents & puzzle,
                                              BoardContents & solution,
                                              Difficulty difficultyRequired,
                                              Symmetry symmetry)
{
    qCDebug(KSudokuLog) << "Entered generatePuzzle(): difficulty "
                        << difficultyRequired << ", symmetry " << symmetry;
    setSeed();

    SudokuType puzzleType = m_graph->specificType();
    if ((puzzleType == Mathdoku) || (puzzleType == KillerSudoku)) {
	// Generate variants of Mathdoku (aka KenKen TM) or Killer Sudoku types.
	int maxTries = 10;
	int numTries = 0;
	bool success = false;
	while (true) {
	    MathdokuGenerator mg (m_graph);
	    // Find numbers to satisfy Sudoku rules: they will be the solution.
	    solution = fillBoard();
	    // Generate a Mathdoku or Killer Sudoku puzzle having this solution.
	    numTries++;
	    success = mg.generateMathdokuTypes (puzzle, solution,
				    &m_KSudokuMoves, difficultyRequired);
	    if (success) {
		return true;
	    }
	    else if (numTries >= maxTries) {
		QWidget owner;
		if (KMessageBox::questionYesNo (&owner,
			    i18n("Attempts to generate a puzzle failed after "
				 "about 200 tries. Try again?"),
			    i18n("Mathdoku or Killer Sudoku Puzzle"))
			    == KMessageBox::No) {
		    return false;	// Go back to the Welcome screen.
		}
		numTries = 0;		// Try again.
	    }
	}
    }
    else {
	// Generate variants of Sudoku (2D) and Roxdoku (3D) types.
	return generateSudokuRoxdokuTypes (puzzle, solution,
                                    difficultyRequired, symmetry);
    }
}

bool SudokuBoard::generateSudokuRoxdokuTypes (BoardContents & puzzle,
                                              BoardContents & solution,
                                              Difficulty difficultyRequired,
                                              Symmetry symmetry)
{
    const int     maxTries = 20;
    int           count = 0;
    float         bestRating = 0.0;
    int           bestDifficulty = 0;
    int           bestNClues = 0;
    int           bestNGuesses = 0;
    int           bestFirstGuessAt = 0;
    BoardContents currPuzzle;
    BoardContents currSolution;

    QTime t;
    t.start();
    if (m_graph->sizeZ() > 1) {
	symmetry = NONE;		// Symmetry not implemented in 3-D.
    }
    if (symmetry == RANDOM_SYM) {	// Choose a symmetry at random.
        symmetry = (Symmetry) (qrand() % (int) LAST_CHOICE);
    }
    qCDebug(KSudokuLog) << "SYMMETRY IS" << symmetry;
    if (symmetry == DIAGONAL_1) {
	// If diagonal symmetry, choose between NW->SE and NE->SW diagonals.
        symmetry = (qrand() % 2 == 0) ? DIAGONAL_1 : DIAGONAL_2;
        qCDebug(KSudokuLog) << "Diagonal symmetry, choosing " <<
            ((symmetry == DIAGONAL_1) ? "DIAGONAL_1" : "DIAGONAL_2");
    }

    while (true) {
        // Fill the board with values that satisfy the Sudoku rules but are
        // chosen in a random way: these values are the solution of the puzzle.
        currSolution = this->fillBoard();
        qCDebug(KSudokuLog) << "Return from fillBoard() - time to fill board:"
                            << t.elapsed() << " msec";

        // Randomly insert solution-values into an empty board until a point is
        // reached where all the cells in the solution can be logically deduced.
        currPuzzle = insertValues (currSolution, difficultyRequired, symmetry);
        qCDebug(KSudokuLog) << "Return from insertValues() - duration:"
                            << t.elapsed() << " msec";

        if (difficultyRequired > m_stats.difficulty) {
            // Make the puzzle harder by removing values at random.
            currPuzzle = removeValues (currSolution, currPuzzle,
                                       difficultyRequired, symmetry);
            qCDebug(KSudokuLog) << "Return from removeValues() - duration:"
                                << t.elapsed() << " msec";
        }

        Difficulty d = calculateRating (currPuzzle, 5);
        count++;
        qCInfo(KSudokuLog) <<  "CYCLE" << count << ", achieved difficulty" << d
                           << ", required" << difficultyRequired << ", rating"
                           << m_accum.rating;

	// Use the highest rated puzzle so far.
	if (m_accum.rating > bestRating) {
	    bestRating       = m_accum.rating;
	    bestDifficulty   = d;
	    bestNClues       = m_stats.nClues;
	    bestNGuesses     = m_accum.nGuesses;
	    bestFirstGuessAt = m_stats.firstGuessAt;
	    solution         = currSolution;
	    puzzle           = currPuzzle;
	}

	// Express the rating to 1 decimal place in whatever locale we have.
	QString ratingStr = ki18n("%1").subs(bestRating, 0, 'f', 1).toString();
	// Check and explain the Sudoku/Roxdoku puzzle-generator's results.
	if ((d < difficultyRequired) && (count >= maxTries)) {
            // Exit after max attempts?
            QWidget owner;
            int ans = KMessageBox::questionYesNo (&owner,
                      i18n("After %1 tries, the best difficulty level achieved "
			   "is %2, with internal difficulty rating %3, but you "
			   "requested difficulty level %4. Do you wish to try "
			   "again or accept the puzzle as is?\n"
			   "\n"
			   "If you accept the puzzle, it may help to change to "
			   "No Symmetry or some low symmetry type, then use "
			   "Game->New and try generating another puzzle.",
			   maxTries, bestDifficulty,
			   ratingStr, difficultyRequired),
                      i18n("Difficulty Level"),
                      KGuiItem(i18n("&Try Again")), KGuiItem(i18n("&Accept")));
            if (ans == KMessageBox::Yes) {
                count = 0;	// Continue on if the puzzle is not hard enough.
                continue;
            }
            break;		// Exit if the puzzle is accepted.
	}
        if ((d >= difficultyRequired) || (count >= maxTries)) {
            QWidget owner;
	    int ans = 0;
	    if (m_accum.nGuesses == 0) {
                ans = KMessageBox::questionYesNo (&owner,
		       i18n("It will be possible to solve the generated puzzle "
			    "by logic alone. No guessing will be required.\n"
			    "\n"
			    "The internal difficulty rating is %1. There are "
			    "%2 clues at the start and %3 moves to go.",
			    ratingStr, bestNClues,
			    (m_stats.nCells - bestNClues)),
		       i18n("Difficulty Level"),
                       KGuiItem(i18n("&OK")), KGuiItem(i18n("&Retry")));
	    }
	    else {
                QString avGuessStr = ki18n("%1").subs(((float) bestNGuesses) /
			5.0, 0, 'f', 1).toString(); // Format as for ratingStr.
                ans = KMessageBox::questionYesNo (&owner,
		       i18n("Solving the generated puzzle will require an "
			    "average of %1 guesses or branch points and if you "
			    "guess wrong, backtracking will be necessary. The "
			    "first guess should come after %2 moves.\n"
			    "\n"
			    "The internal difficulty rating is %3, there are "
			    "%4 clues at the start and %5 moves to go.",
			    avGuessStr, bestFirstGuessAt, ratingStr,
			    bestNClues, (m_stats.nCells - bestNClues)),
                       i18n("Difficulty Level"),
                       KGuiItem(i18n("&OK")), KGuiItem(i18n("&Retry")));
	    }
	    // Exit when the required difficulty or number of tries is reached.
            if (ans == KMessageBox::No) {
                count = 0;
                bestRating = 0.0;
                bestDifficulty = 0;
                bestNClues = 0;
                bestNGuesses = 0;
                bestFirstGuessAt = 0;
                continue;	// Start again if the user rejects this puzzle.
            }
	    break;		// Exit if the puzzle is OK.
        }
    }

    qCDebug(KSudokuLog) << "FINAL PUZZLE" << puzzle;

    return true;
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
        dbo1 "SOLVE PUZZLE %d\n", n);
        solution = solveBoard (puzzle, nSamples == 1 ? NotRandom : Random);
        dbo1 "PUZZLE SOLVED %d\n", n);
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
    dbo1 "  Av guesses %2.1f  Av deduces %2.1f"
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

void SudokuBoard::getMoveList (QList<int> & moveList)
{
    moveList = m_KSudokuMoves;
}

BoardContents & SudokuBoard::solveBoard (const BoardContents & boardValues,
                                               GuessingMode gMode)
{
    qCInfo(KSudokuLog) << "solveBoard()" << boardValues;
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
            // NOTE: We keep the stack of states.  It is needed by checkPuzzle()
	    //       for the multiple-solutions test and deleted when its parent
	    //       SudokuBoard object (i.e. this->) is deleted.
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

            deduceValues (filled, Random /* NotRandom */);
            qCDebug(KSudokuLog) << "Puzzle:" << puzzle << "; filled" << filled;
        }
    }
    qCDebug(KSudokuLog) << "Puzzle:" << puzzle;

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
    qCDebug(KSudokuLog) << "Puzzle:" << puzzle;
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
                qCInfo(KSudokuLog) << "Break on difficulty - replaced" << value
                                   << "at cell" << cell << ", overshoot is"
                                   << tailOfRemoved.count();
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
    m_KSudokuMoves.clear();
    Move m;
    Move mType;
    while (! m_moves.isEmpty()) {
        m = m_moves.takeFirst();
        mType = m_moveTypes.takeFirst();
	int val = pairVal(m);
	int pos = pairPos(m);
	int row = m_graph->cellPosY (pos);
	int col = m_graph->cellPosX (pos);

        switch (mType) {
        case Single:
            dbo2 "  Single Pick %d %d row %d col %d\n", val, pos, row+1, col+1);
	    m_KSudokuMoves.append (pos);
            s.nSingles++;
            break;
        case Spot:
            dbo2 "  Single Spot %d %d row %d col %d\n", val, pos, row+1, col+1);
	    m_KSudokuMoves.append (pos);
            s.nSpots++;
            break;
        case Deduce:
            dbo2 "Deduce: Iteration %d\n", m);
            s.nDeduces++;
            break;
        case Guess:
            dbo2 "GUESS:        %d %d row %d col %d\n", val, pos, row+1, col+1);
	    m_KSudokuMoves.append (pos);
            if (s.nGuesses < 1) {
                s.firstGuessAt = s.nSingles + s.nSpots + 1;
            }
            s.nGuesses++;
            break;
        case Wrong:
            dbo2 "WRONG GUESS:  %d %d row %d col %d\n", val, pos, row+1, col+1);
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

    dbo1 "  aM: Type %2d %2d: clues %3d %3d %2.1f%%   %3dP %3dS %3dG "
         "%3dM %3dD %3.1fR D=%d F=%d\n\n",
         m_stats.type, m_stats.order,
         s.nClues, s.nCells, ((float) s.nClues / s.nCells) * 100.0,
         s.nSingles, s.nSpots, s.nGuesses, (s.nSingles + s.nSpots + s.nGuesses),
         s.nDeduces, s.rating, s.difficulty, s.firstGuessAt);
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

    int depth = m_graph->sizeZ();	// If 2-D, depth == 1, else depth > 1.
    for (int k = 0; k < depth; k++) {
      int z = (depth > 1) ? (depth - k - 1) : k;
      for (int j = 0; j < m_graph->sizeY(); j++) {
	if ((j != 0) && (j % m_blockSize == 0)) {
	    printf ("\n");		// Gap between square blocks.
	}
        int y = (depth > 1) ? (m_graph->sizeY() - j - 1) : j;
        for (int x = 0; x < m_graph->sizeX(); x++) {
            index = m_graph->cellIndex (x, y, z);
            value = boardValues.at (index);
            if (x % m_blockSize == 0) {
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
      printf ("\n");			// Next Z or end of 2D puzzle/solution.
    }
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
                dbo3 "Cell %d, valid numbers %03o\n", cell, numbers);
                if (numbers == 0) {
                    dbo2 "SOLUTION FAILED: RETURN at cell %d\n", cell);
                    return solutionFailed (guesses);
                }
                int validNumber = 1;
                while (numbers != 0) {
                    dbo3 "Numbers = %03o, validNumber = %d\n",
                            numbers, validNumber);
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
	    QVector<int> cellList = m_graph->clique (group);
            qint32 numbers = m_requiredGroupValues.at (group);
            dbo3 "Group %d, valid numbers %03o\n", group, numbers);
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
			cell = cellList.at (n);
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

void SudokuBoard::clear (BoardContents & boardValues)
{
    boardValues = m_graph->emptyBoard();	// Set cells vacant or unusable.
}

BoardContents & SudokuBoard::fillBoard()
{
    // Solve the empty board, thus filling it with values at random.  These
    // values can be the starting point for generating a puzzle and also the
    // final solution of that puzzle.

    clear (m_currentValues);

    // Fill a central block with values 1 to m_order in random sequence.  This
    // reduces the solveBoard() time considerably, esp. if blockSize is 4 or 5.
    QVector<int> sequence (m_order);
    QVector<int> cellList = m_graph->clique (m_nGroups / 2);
    randomSequence (sequence);
    for (int n = 0; n < m_order; n++) {
        m_currentValues [cellList.at (n)] = sequence.at (n) + 1;
    }

    solveBoard (m_currentValues);
    dbo1 "BOARD FILLED\n");
    return m_currentValues;
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

    dbo2 "Enter setUpValueRequirements()\n");
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
	dbo3 "Group %3d ", group);
	QVector<int> cellList = m_graph->clique (group);
        bitPattern = 0;
        for (int n = 0; n < m_groupSize; n++) {
            int value = boardValues.at (cellList.at (n)) - 1;
            if (value != m_unusable) {
                bitPattern |= (1 << value);	// Add bit for each value found.
            }
	    dbo3 "%3d=%2d ", cellList.at (n), value + 1);
            index++;
        }
        // Reverse all the bits, giving values currently not found in the group.
        m_requiredGroupValues [group] = bitPattern ^ allValues;
	dbo3 "bits %03o\n", m_requiredGroupValues.at (group));
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
	QVector<int> cellList = m_graph->clique (group);
        for (int n = 0; n < m_order; n++) {
	    int cell = cellList.at (n);
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
    QList<int> groupList = m_graph->cliqueList(cell);
    foreach (int group, groupList) {
        m_requiredGroupValues [group] &= bitPattern;

	QVector<int> cellList = m_graph->clique (group);
        for (int n = 0; n < m_order; n++) {
	    int cell = cellList.at (n);
            m_validCellValues [cell] &= bitPattern;
        }
    }
}

void SudokuBoard::changeClues (BoardContents & to, int cell, Symmetry type,
                               const BoardContents & from)
{
    int nSymm = 1;
    int indices[8];
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
    out[0]     = index;
    if (type == NONE) {
	return result;
    }

    int row    = m_graph->cellPosY (index);
    int col    = m_graph->cellPosX (index);
    int lr     = size - col - 1;		// For left-to-right reflection.
    int tb     = size - row - 1;		// For top-to-bottom reflection.

    switch (type) {
        case DIAGONAL_1:
	    // Reflect a copy of the point around two central axes making its
	    // reflection in the NW-SE diagonal the same as for NE-SW diagonal.
            row = tb;
            col = lr;
            // No break; fall through to case DIAGONAL_2.
        case DIAGONAL_2:
	    // Reflect (col, row) in the main NW-SE diagonal by swapping coords.
            out[1] = m_graph->cellIndex(row, col);
            result = (out[1] == out[0]) ? 1 : 2;
            break;
        case CENTRAL:
            out[1] = (size * size) - index - 1;
            result = (out[1] == out[0]) ? 1 : 2;
            break;
	case SPIRAL:
	    if ((size % 2 != 1) || (row != col) || (col != (size - 1)/2)) {
		result = 4;			// This is not the central cell.
		out[1] = m_graph->cellIndex(lr,  tb);
		out[2] = m_graph->cellIndex(row, lr);
		out[3] = m_graph->cellIndex(tb,  col);
	    }
            break;
        case FOURWAY:
	    out[1] = m_graph->cellIndex(row, col);	// Interchange X and Y.
	    out[2] = m_graph->cellIndex(lr,  row);	// Left-to-right.
	    out[3] = m_graph->cellIndex(row, lr);	// Interchange X and Y.
	    out[4] = m_graph->cellIndex(col, tb);	// Top-to-bottom.
	    out[5] = m_graph->cellIndex(tb,  col);	// Interchange X and Y.
	    out[6] = m_graph->cellIndex(lr,  tb);	// Both L-R and T-B.
	    out[7] = m_graph->cellIndex(tb,  lr);	// Interchange X and Y.

	    int k;
	    for (int n = 1; n < 8; n++) {
		for (k = 0; k < result; k++) {
		    if (out[n] == out[k]) {
			break;				// Omit duplicates.
		    }
		}
		if (k >= result) {
		    out[result] = out[n];
		    result++;				// Use unique positions.
		}
	    }
            break;
        case LEFT_RIGHT:
	    out[1] = m_graph->cellIndex(lr,  row);
            result = (out[1] == out[0]) ? 1 : 2;
            break;
        default:
            break;
    }
    return result;
}


