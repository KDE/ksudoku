/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006      Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2007 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
 *   Copyright 2015      Ian Wadham <iandw.au@gmail.com>                   *
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
#include <QString>
#include <QUrl>

class SKGraph;
class QDomElement;
class QWidget;

namespace ksudoku {

class Game;
class Puzzle;
class HistoryEvent;
class Serializer {
public:
	static SKGraph* loadCustomShape
		    (const QUrl& url, QWidget* window, QString& errorMsg);
	static bool store
		    (const Game& game, const QUrl& url, QWidget* window, QString& errorMsg);
	static Game load
		    (const QUrl& url, QWidget* window, QString& errorMsg);

private:
	// TODO - IDW. Maybe there should be shared methods for file handling.
	//             And do all these methods need to be static?
	static Game deserializeGame(const QDomElement &element);
	static Puzzle* deserializePuzzle(const QDomElement &element) ;
	static SKGraph* deserializeGraph(const QDomElement &element);
	static bool deserializeClique(SKGraph * graph, const QString & size,
						       const QString & text);
	static bool deserializeCage(SKGraph * graph, const QDomElement & e);
	static QList<HistoryEvent> deserializeHistory(const QDomElement &element);
	static HistoryEvent deserializeSimpleHistoryEvent(const QDomElement &element);
	static HistoryEvent deserializeComplexHistoryEvent(const QDomElement &element);
	


	static bool serializeGame(QDomElement& parent, const Game& game);
	static bool serializePuzzle(QDomElement& parent, const Puzzle* puzzle);
	static bool serializeGraph(QDomElement& parent, const SKGraph* graph);
	static bool serializeHistory(QDomElement& parent, const Game& game);
	static bool serializeHistoryEvent(QDomElement& parent, const HistoryEvent& event);
	
};

}

#endif
