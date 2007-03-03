#ifndef _KSUDOKUGAME_H_
#define _KSUDOKUGAME_H_

#include "ksudoku_types.h"
#include <qobject.h>
#include "history.h"


class SKPuzzle;
class SKSolver;
class KUrl;

namespace ksudoku {

class Puzzle ;
class Symbols;

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
	void cellChange(uint index);
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
	
	///convert coordinates in a puzzle to one index value
	///no bound checks are performed for performance reason
	uint index(uint x, uint y, uint z = 0) const;
	
	/**
	 * The nubmer of cells of the puzzle
	 */
	uint size() const;
	
	/**
	 * Checks wheter a set contains obvious errors
	 *
	 * @returns True when no error against the rules of the graph exists. This doesn't mean,
	 *          that the values are suitable to solve the puzzle.
	 */
	bool simpleCheck() const;
	
	/**
	 * Use this to show which cells might not be used for a value.
	 * @param[in] val       The value against which the puzzle should be highlighted
	 * @param[in] allValues Whether cells which allready have a value should also be hightlighted
	 * @return A array of bools for each cell. True when the cell might not be used otherwise false.
	 */
	QBitArray highlightValueConnections(uint val, bool allValues = false) const;

	///@return pointer to current puzzle
	Puzzle* puzzle() const;
	///@return poiter to symbols used for current puzzle
	Symbols const* symbols() const;
	
	/**
	 * Gets the the interface of the game. Use this if for connecting to signals or
	 * slots of the game
	 */
	GameIFace* interface();
	
	Game& operator=(const Game& game);
	
public:
	
	bool hasSolver();
	
	uint order() const;

	bool marker(uint index, uint value) const;
	inline bool marker(uint value, uint x, uint y, uint z = 0) const;
	uint value(uint index) const;
	inline uint value(uint x, uint y, uint z = 0) const;
	bool given(uint index) const;
	inline bool given(uint x, uint y, uint z = 0) const;
	
	/**
	 * Returns the state of a cell
	 * @param[in] index The index of the cell
	 */
	ksudoku::ButtonState buttonState(uint index) const;
	inline ksudoku::ButtonState buttonState(uint x, uint y, uint z = 0) const;
	CellInfo cellInfo(uint index) const;
	inline CellInfo cellInfo(uint x, uint y, uint z = 0) const;
	
	/**
	 * Sets one marker in a cell
	 * @param[in] index The index of the cell
	 * @param[in] val   The value of the marker
	 * @param[in] state Whether the marker shoudl be set or unset
	 * @return Whether this function was executed succesfully
	 */
	bool setMarker(uint index, uint val, bool state);
	inline bool setMarker(uint val, bool state, uint x, uint y, uint z = 0);
	inline bool flipMarker(uint index, uint val);
	inline bool flipMarker(uint val, uint x, uint y, uint z = 0);
	
	/**
	 * @brief Sets the value of a cell
	 * @param[in] index The index of the cell
	 * @param[in] val   The new value of the cell
	 */
	void setValue(uint index, uint val);
	inline void setValue(uint val, uint x, uint y, uint z = 0);
	
	/**
	 * Sets whether cell @p index is @p given (A given cell is not changeable by the player).
	 */
	void setGiven(uint index, bool given);
	inline void setGiven(bool given, uint x, uint y, uint z = 0);
	
	/**
	 * Gets the all current values of the game
	 */
	const QByteArray allValues() const;
	
	/**
	 * @see ksudoku::Puzzle::value2Char
	 */
	QChar value2Char(uint value) const;
	
	/**
	 * @see ksudoku::Puzzle:char2Value
	 */
	int char2Value(QChar c) const;
	
	/**
	 * Gives one value in a randomly choosen cell.
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
	 * Sets whether the user had requested some hint.
	 * @note This method is for loading/saving only
	 * @see userHadHelp(), giveHint(), autoSolve()
	 */
	void setUserHadHelp(bool hadHelp);
	
	// History
	
	bool canUndo() const;
	bool canRedo() const;
	bool canAddCheckpoint() const;
	bool canUndo2Checkpoint() const;
	
	/**
	 * Adds an event to the history and performs it
	 */
	void doEvent(const HistoryEvent& event);
	
	/**
	 * Returns count of history events
	 */
	uint historyLength() const;
	
	/**
	 * Returns the hitory event at position @p i
	 */
	HistoryEvent historyEvent(uint i) const;
	
	
private:
	/**
	 * When the game was finished this function emits a @c completed()
	 */
	void checkCompleted();
	
private:
	class Private;
	
	Private* m_private;
};

inline bool Game::marker(uint val, uint x, uint y, uint z) const {
	return marker(index(x, y, z), val);
}

inline bool Game::setMarker(uint val, bool state, uint x, uint y, uint z) {
	return setMarker(index(x, y, z), val, state);
}

inline bool Game::flipMarker(uint index, uint val) {
	return setMarker(index, val, !marker(index, val));
}

inline bool Game::flipMarker(uint val, uint x, uint y, uint z) {
	return flipMarker(index(x, y, z), val);
}

inline uint Game::value(uint x, uint y, uint z) const {
	return value(index(x, y, z));
}

inline void Game::setValue(uint val, uint x, uint y, uint z) {
	setValue(index(x, y, z), val);
}

inline bool Game::given(uint x, uint y, uint z) const {
	return given(index(x, y, z));
}

inline void Game::setGiven(bool given, uint x, uint y, uint z) {
	setGiven(index(x, y, z), given);
}

inline ksudoku::ButtonState Game::buttonState(uint x, uint y, uint z) const {
	return buttonState(index(x, y, z));
}

inline CellInfo Game::cellInfo(uint x, uint y, uint z) const {
	return cellInfo(index(x, y, z));
}

}

#endif
