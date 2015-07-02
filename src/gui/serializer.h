/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006      Mick Kappenburg <ksudoku@kappendburg.net>         *
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

#ifndef _KSUDOKUSERIALIZER_H_
#define _KSUDOKUSERIALIZER_H_

#include <QList>

class SKGraph;
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
	static SKGraph* loadCustomShape
		    (const KUrl& url, QWidget* window, QString* errorMsg = 0);
	static bool store
		    (const Game& game, const KUrl& url, QWidget* window);
	static Game load
		    (const KUrl& url, QWidget* window, QString* errorMsg = 0);

private:
	// TODO - IDW. Maybe there should be shared methods for file handling.
	//             And do all these methods need to be static?
	static Game deserializeGame(QDomElement element);
	static Puzzle* deserializePuzzle(QDomElement element) ;
	static SKGraph* deserializeGraph(QDomElement element);
	static bool deserializeClique(SKGraph * graph, const QString & size,
						       const QString & text);
	static bool deserializeCage(SKGraph * graph, const QDomElement & e);
	static QList<HistoryEvent> deserializeHistory(QDomElement element);
	static HistoryEvent deserializeSimpleHistoryEvent(QDomElement element);
	static HistoryEvent deserializeComplexHistoryEvent(QDomElement element);
	


	static bool serializeGame(QDomElement& parent, const Game& game);
	static bool serializePuzzle(QDomElement& parent, const Puzzle* puzzle);
	static bool serializeGraph(QDomElement& parent, const SKGraph* graph);
	static bool serializeHistory(QDomElement& parent, const Game& game);
	static bool serializeHistoryEvent(QDomElement& parent, const HistoryEvent& event);
	
};

}

#endif
