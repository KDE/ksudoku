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

#ifndef _KSUDOKUGAME_H_
#define _KSUDOKUGAME_H_

// #include "ksudoku_types.h" // TODO: IDW, remove this -- now redundant.
#include <qobject.h>
#include "history.h"


class KUrl;

namespace ksudoku {

class Puzzle ;

/**
 * The interface of a game. Since the game itself is a shared class you cannot connect to
 * the game. Use game->interface() to get a interface instance which you can use to connect
 * to.
 */
class GameIFace : public QObject {
	Q_OBJECT

public slots:
	virtual void undo() = 0;
	virtual void redo() = 0;
	virtual void addCheckpoint() = 0;
	virtual void undo2Checkpoint() = 0;
	
signals:
	void modified(bool isModified);
	void completed(bool isCorrect, const QTime& required, bool withHelp);
	void cellChange(int index);
	void fullChange();
};

/**
 * @author Johannes Bergmeier
 *
 * A Game instance represents a interactive game.
 *
 * For simple puzzles use @c ksudoku::Puzzle.
 *
 * @see ksudoku::Puzzle
 */
class Game {
public:
	/**
	 * Creates an invalid game
	 */
	Game();
	
	/**
	 * @param[in] puzzle The puzzle for this game
	 * @note The Game takes ownership on @p puzzle
	 */
	explicit Game(Puzzle* puzzle);
	
	/**
	 * Copy constructor
	 */
	Game(const Game& game);
	~Game();
	
public:
	// TODO improve this
	inline bool isValid() const { return static_cast<bool>(m_private); }
	
	/**
	 * The nubmer of cells of the puzzle
	 */
	int size() const;
	
	/**
	 * Checks whether a set contains obvious errors
	 *
	 * @returns True when no error against the rules of the graph exists. This doesn't mean,
	 *          that the values are suitable to solve the puzzle.
	 */
	bool simpleCheck() const;
	
	///@return pointer to current puzzle
	Puzzle* puzzle() const;
	
	/**
	 * Gets the interface of the game. Use this if for connecting to signals or
	 * slots of the game
	 */
	GameIFace* interface() const;
	
	Game& operator=(const Game& game);
	
public:
	int order() const;

	int  value(int index) const;
	int  solution(int index) const;
	bool given(int index) const;
	bool marker(int index, int value) const;
	
	/**
	 * Returns the state of a cell
	 * @param[in] index The index of the cell
	 */
	ksudoku::ButtonState buttonState(int index) const;
	CellInfo cellInfo(int index) const;
	
	/**
	 * Sets one marker in a cell
	 * @param[in] index The index of the cell
	 * @param[in] val   The value of the marker
	 * @param[in] state Whether the marker shoudl be set or unset
	 * @return Whether this function was executed successfully
	 */
	bool setMarker(int index, int val, bool state);
	inline bool flipMarker(int index, int val);
	
	/**
	 * @brief Sets the value of a cell
	 * @param[in] index The index of the cell
	 * @param[in] val   The new value of the cell
	 */
	void setValue(int index, int val);
	
	/**
	 * Sets whether cell @p index is @p given (A given cell is not changeable by the player).
	 */
	void setGiven(int index, bool given);
	
	/**
	 * Gets the all current values of the game
	 */
	const QByteArray allValues() const;
	
	/**
	 * Gives one value in a randomly chosen cell.
	 */
	bool giveHint();

	/**
	 * Makes the whole puzzle given.
	 */
	bool autoSolve();
	
	/**
	 * Returns the time since game start
	 */
	QTime time() const;
	
	/**
	 * Sets the URL. Game itself doesn't use the URL, but remembers it for other users.
	 */
	void setUrl(const KUrl& url);
	
	/**
	 * Gets the URL. Game itself doesn't use the URL, but remembers it for other users.
	 */
	KUrl getUrl() const;
	
	/**
	 * Returns whether the user requested some hint.
	 * @see giveHint(), autoSolve()
	 */
	bool userHadHelp() const;

	/**
	 * Returns whether the game was allready solved.
	 */
	bool wasFinished() const;
	
	/**
	 * Sets whether the user had requested some hint.
	 * @note This method is for loading/saving only
	 * @see userHadHelp(), giveHint(), autoSolve()
	 */
	void setUserHadHelp(bool hadHelp);
	
	// History
	
	bool canUndo() const;
	bool canRedo() const;
	
	/**
	 * Adds an event to the history and performs it
	 */
	void doEvent(const HistoryEvent& event);
	
	/**
	 * Returns count of history events
	 */
	int historyLength() const;
	
	/**
	 * Returns the hitory event at position @p i
	 */
	HistoryEvent historyEvent(int i) const;
	
	
private:
	/**
	 * When the game was finished this function emits a @c completed()
	 */
	void checkCompleted();
	
private:
	class Private;
	
	Private* m_private;
};

inline bool Game::flipMarker(int index, int val) {
	return setMarker(index, val, !marker(index, val));
}

}

#endif
