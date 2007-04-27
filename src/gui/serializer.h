#ifndef _KSUDOKUSERIALIZER_H_
#define _KSUDOKUSERIALIZER_H_

#include <QList>

class SKSolver;
class QDomElement;
class KUrl;
class QWidget;
class QString;

namespace ksudoku {

class Game;
class Puzzle;
class HistoryEvent;
class Serializer {
public:
	static Game deserializeGame(QDomElement element);
	static Puzzle* deserializePuzzle(QDomElement element) ;
	static SKSolver* deserializeGraph(QDomElement element);
	static QList<HistoryEvent> deserializeHistory(QDomElement element);
	static HistoryEvent deserializeSimpleHistoryEvent(QDomElement element);
	static HistoryEvent deserializeComplexHistoryEvent(QDomElement element);
	
	static Game load(const KUrl& url, QWidget* window, QString* errorMsg = 0);
	static SKSolver* loadCustomShape(const KUrl& url, QWidget* window, QString* errorMsg = 0);


	static bool serializeGame(QDomElement& parent, const Game& game);
	static bool serializePuzzle(QDomElement& parent, const Puzzle* puzzle);
	static bool serializeGraph(QDomElement& parent, const SKSolver* solver);
	static bool serializeHistory(QDomElement& parent, const Game& game);
	static bool serializeHistoryEvent(QDomElement& parent, const HistoryEvent& event);
	
	static bool store(const Game& game, const KUrl& url, QWidget* window);
	static bool storeCustomShape(const SKSolver* solver, const KUrl& url, QWidget* window);
};

}

#endif
