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

#include "serializer.h"
#include "ksudokugame.h"
#include "puzzle.h"

#include <qdom.h>
//Added by qt3to4:
#include <QList>
#include <QTextStream>
#include <kurl.h>
#include <ktemporaryfile.h>
#include <kio/netaccess.h>
#include <qfile.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "ksudoku.h"
#include "symbols.h"

namespace ksudoku {

const char *     typeNames[] = {"Plain", "XSudoku", "Jigsaw",
				"Samurai", "TinySamurai", "Roxdoku"};
const SudokuType types[]     = {Plain, XSudoku, Jigsaw,
				Samurai, TinySamurai, Roxdoku};

Game Serializer::deserializeGame(QDomElement element) {
	bool hasPuzzle = false;
	Puzzle* puzzle = 0;
	bool hasHistory = false;
	QList<HistoryEvent> history;
	
	bool hadHelp = static_cast<bool>(element.attribute("had-help", "0").toInt());
	
	QDomNode child = element.firstChild();
	while (!child.isNull()) {
		if(child.isElement()) {
			if(child.nodeName() == "puzzle") {
				if(hasPuzzle) {
					delete puzzle;
					return Game();
				}
					
				puzzle = deserializePuzzle(child.toElement());
				hasPuzzle = true;
			} else if(child.nodeName() == "history") {
				if(hasHistory) {
					delete puzzle;
					return Game();
				}
				
				history = deserializeHistory(child.toElement());
				hasHistory = true;
			}
		}
		child = child.nextSibling();
	}
	
	if(!puzzle) return Game();
	
	Game game(puzzle);
	game.setUserHadHelp(hadHelp);
	
	if(hasHistory) {
		for(int i = 0; i < history.count(); ++i)
			game.doEvent(history[i]);
	}
	
	return game;
}

Puzzle* Serializer::deserializePuzzle(QDomElement element) {
	bool hasGraph = false;
	SKGraph* graph = 0;
	bool hasValues = false;
	QString valuesStr;
	bool hasSolution = false;
	QString solutionStr;
	
	QString content;
	QDomNode child = element.firstChild();
	while (!child.isNull()) {
		if(child.isElement()) {
			if(child.nodeName() == "graph") {
				if(hasGraph) {
					delete graph;
					return 0;
				}
					
				graph = deserializeGraph(child.toElement());
				hasGraph = true;
			} else if(child.nodeName() == "values") {
				if(hasValues) {
					delete graph;
					return 0;
				}
				
				valuesStr = child.toElement().text();
				hasValues = true;
			} else if(child.nodeName() == "solution") {
				// TODO remove deserialization of solution, it is no longer required
				if(hasSolution) {
					delete graph;
					return 0;
				}
				
				solutionStr = child.toElement().text();
				hasSolution = true;
			}
		}
		child = child.nextSibling();
	}
	
	if(!graph) return 0;
	if(valuesStr.length() != graph->size()) {
		delete graph;
		return 0;
	}
	// TODO remove deserialization of solution, it is no longer required
	if(solutionStr.length() != 0 && solutionStr.length() != graph->size()) {
		delete graph;
		return 0;
	}
	
	Puzzle* puzzle = new Puzzle(graph, hasSolution);
	
	QByteArray values;
	values.resize(graph->size());
	for(int i = 0; i < graph->size(); ++i) {
		values[i] = Symbols::ioSymbol2Value(valuesStr[i]);
	}
	
	// TODO remove deserialization of solution, it is no longer required
	QByteArray solution;
	if(solutionStr.length() != 0) {
		solution.resize(graph->size());
		for(int i = 0; i < graph->size(); ++i) {
			solution[i] = Symbols::ioSymbol2Value(solutionStr[i]);
		}
	}
	
	puzzle->init(values);
	return puzzle;
}

static int readInt(QDomElement element, const QString& name, int* err)
{ //out of class, cannot be static
	*err = 1;
	QString Str = element.attribute(name);
	if(Str.isNull())
		return 0;
	bool noFailure = true;
	int num = Str.toInt(&noFailure, 0);
	if(!noFailure)
		return 0;
	*err = 0;
	return num;
}

SKGraph* Serializer::deserializeGraph(QDomElement element) {
	bool noFailure = true;
	
	QString orderStr = element.attribute("order");
	if(orderStr.isNull())
		return 0;
	int order = orderStr.toInt(&noFailure, 0);
	if(!noFailure)
		return 0;
	
	QString type = element.attribute("type");
	if(type.isNull())
		return 0;
	
	bool d3 = false;
	if(type == "sudoku") {
		GraphSudoku *graph = new GraphSudoku(order);
		graph->init();
		return graph;
	} else if(type == "roxdoku") {
		GraphRoxdoku *graph = new GraphRoxdoku(order);
		graph->init();
		return graph;
	} else if(type == "custom") {
		int err=0;
		int ncliques = readInt(element,"ncliques", &err);
		int sizeX = readInt(element,"sizeX",&err);
		int sizeY = readInt(element,"sizeY",&err);
		int sizeZ = readInt(element,"sizeZ",&err);
		//int size = sizeX*sizeY*sizeZ;

		QString name = element.attribute("name");
		QString typeName = element.attribute("specific-type");
		SudokuType puzzleType = Plain; // Default puzzle-type.
	        for (int n = 0; n < EndSudokuTypes; n++) {
		    QString lookup = QString (typeNames [n]);
	            if (QString::compare (typeName, lookup, Qt::CaseInsensitive)
			== 0) {
		        puzzleType = types [n];
	                break;
	            }
		}

		if(err==1) return 0;
		if(sizeX<1 || sizeY<1 || sizeZ<1) return 0;

		QString cliques = "";
		QDomNode child = element.firstChild();
		while (!child.isNull()) 
		{
			if(child.isElement())
			{
				QString sz = child.toElement().attribute("size");
				if(sz.isNull()) return 0;
				cliques += sz + ' ' + child.toElement().text();
			}
			child = child.nextSibling();
		}
		
		GraphCustom* graph = new GraphCustom();
		graph->init(name.toLatin1(), puzzleType, order,
			    sizeX, sizeY, sizeZ, ncliques, cliques.toLatin1());
		if(graph->valid==false) return 0;
		return graph;
	}
	
	return 0;
}

QList<HistoryEvent> Serializer::deserializeHistory(QDomElement element) {
	QList<HistoryEvent> history;
	
	QDomNode child = element.firstChild();
	while (!child.isNull()) {
		if(child.isElement()) {
			if(child.nodeName() == "simple-event") {
				history.append(deserializeSimpleHistoryEvent(child.toElement()));
			} else if(child.nodeName() == "complex-event") {
				history.append(deserializeComplexHistoryEvent(child.toElement()));
			}
		}
		child = child.nextSibling();
	}
	return history;
}

HistoryEvent Serializer::deserializeSimpleHistoryEvent(QDomElement element) {
	QString indexStr = element.attribute("index");
	QString markerStr = element.attribute("markers");
	QString valueStr = element.attribute("value");
	bool given = element.attribute("given") == "true";
	bool noFailure = true;
	
	int index = indexStr.toInt(&noFailure, 0);
	if(!noFailure)
		return HistoryEvent();
	
	
	if(markerStr.isNull() == valueStr.isNull())
		return HistoryEvent();
	
	
	if(!markerStr.isNull()) {
		QBitArray markers(markerStr.length());
		for(int i = 0; i < markerStr.length(); ++i)
			markers[i] = markerStr[i] != '0';
		
		return HistoryEvent(index, CellInfo(markers));
	} else {
		int value = valueStr.toInt(&noFailure, 0);
		if(!noFailure)
			return HistoryEvent();

		if(given) {
			return HistoryEvent(index, CellInfo(GivenValue, value));
		} else {
			return HistoryEvent(index, CellInfo(CorrectValue, value));
		}
	}
	
	return HistoryEvent();
}

HistoryEvent Serializer::deserializeComplexHistoryEvent(QDomElement /*element*/) {
	// TODO implement this
	return HistoryEvent();
}

SKGraph *Serializer::loadCustomShape(const KUrl &url, QWidget* window, QString *errorMsg) {
	if ( url.isEmpty() ) return 0;
	QString tmpFile;
	bool success = false;
	QDomDocument doc;
	if(KIO::NetAccess::download(url, tmpFile, window) ) {
		QFile file(tmpFile);
		if(file.open(QIODevice::ReadOnly)) {
			int errorLine;
			if(!doc.setContent(&file, 0, &errorLine)) {
				if(errorMsg)
					*errorMsg = i18n("Cannot read XML file on line %1", errorLine);

				return 0;
			}
			success = true;
		}
		KIO::NetAccess::removeTempFile(tmpFile);
	}
	if ( !success ) {
		if(errorMsg)
			*errorMsg = i18n("Cannot load file.");
		return 0;
	}
	
	QDomNode child = doc.documentElement().firstChild();
	while (!child.isNull()) {
		if(child.isElement()) {
			if(child.nodeName() == "graph") {
				return deserializeGraph(child.toElement());
			}
		}
		child = child.nextSibling();
	}
	
	return 0;
}

Game Serializer::load(const KUrl& url, QWidget* window, QString *errorMsg) {
	if ( url.isEmpty() ) return Game();
	QString tmpFile;
	bool success = false;
	QDomDocument doc;
	if(KIO::NetAccess::download(url, tmpFile, window) ) {
		QFile file(tmpFile);
		if(file.open(QIODevice::ReadOnly)) {
			int errorLine;
			if(!doc.setContent(&file, 0, &errorLine)) {
				if(errorMsg)
					*errorMsg = i18n("Cannot read XML file on line %1", errorLine);
				return Game();
			}
			success = true;
		}
		KIO::NetAccess::removeTempFile(tmpFile);
	}
	if ( !success ) {
		if(errorMsg)
			*errorMsg = i18n("Cannot load file.");
		return Game();
	}
	
	// used to ensure, that there is only one game
	bool hasGame = false;
	Game game;
	
	QDomNode child = doc.documentElement().firstChild();
	while (!child.isNull()) {
		if(child.isElement()) {
			if(child.nodeName() == "game") {
				if(hasGame)
					return Game();
				
				game = deserializeGame(child.toElement());
				hasGame = true;
			}
		}
		child = child.nextSibling();
	}

	return game;
}

bool Serializer::serializeGame(QDomElement& parent, const Game& game) {
	QDomElement element = parent.ownerDocument().createElement("game");
	element.setAttribute("had-help", game.userHadHelp());
	serializePuzzle(element, game.puzzle());
	serializeHistory(element, game);
	parent.appendChild(element);
	return true;
}

bool Serializer::serializePuzzle(QDomElement& parent, const Puzzle* puzzle) {
	QString contentStr;
	
	QDomDocument doc = parent.ownerDocument();
	QDomElement element = doc.createElement("puzzle");
	serializeGraph(element, puzzle->graph());
	
	for(int i = 0; i < puzzle->size(); ++i) {
		contentStr += Symbols::ioValue2Symbol(puzzle->value(i));
	}
	
	QDomElement content = doc.createElement("values");
	content.appendChild(doc.createTextNode(contentStr));
	element.appendChild(content);
	
	if(puzzle->hasSolution()) {
		contentStr = QString();
		for(int i = 0; i < puzzle->size(); ++i) {
			contentStr += Symbols::ioValue2Symbol(puzzle->solution(i));
		}
		content = doc.createElement("solution");
		content.appendChild(doc.createTextNode(contentStr));
		element.appendChild(content);
	}
	
	parent.appendChild(element);
	return true;
}

bool Serializer::serializeGraph(QDomElement &parent, const SKGraph *graph) {
	QDomElement element = parent.ownerDocument().createElement("graph");
	element.setAttribute("order", graph->order());
	//element.setAttribute("size", puzzle->size());
	
	GameType type = graph->type();
	element.setAttribute("type" , (type == TypeSudoku) ? "sudoku" : (type == TypeRoxdoku) ? "roxdoku" : "custom");
	if(type == TypeCustom)
	{
		GraphCustom* g = (GraphCustom*) graph;
		element.setAttribute("ncliques", (int) g->cliqueCount());
		element.setAttribute("name", g->name);
		element.setAttribute("sizeX", g->sizeX());
		element.setAttribute("sizeY", g->sizeY());
		element.setAttribute("sizeZ", g->sizeZ());

		for(int i=0; i < g->cliqueCount(); i++)
		{
			QDomElement clique = parent.ownerDocument().createElement("clique");
			clique.setAttribute("size",  (int) g->clique(i).size());
			//serialize clique
			QString contentStr = "";
			for(int j=0; j < g->clique(i).size(); j++)
			{
				contentStr += QString::number(g->clique(i).at(j)) + ' ';
			}
			clique.appendChild(parent.ownerDocument().createTextNode(contentStr));
			element.appendChild(clique);
		}
	}

	parent.appendChild(element);
	return true;
}

bool Serializer::serializeHistory(QDomElement& parent, const Game& game) {
	QDomElement element = parent.ownerDocument().createElement("history");
	
	for(int i = 0; i < game.historyLength(); ++i) {
		if(!serializeHistoryEvent(element, game.historyEvent(i)))
			return false;
	}
	
	parent.appendChild(element);
	return true;
}

bool Serializer::serializeHistoryEvent(QDomElement& parent, const HistoryEvent& event) {
	QDomElement element;
	 
	const QVector<int>& indices = event.cellIndices();
	const QVector<CellInfo>& changes = event.cellChanges();
	
	if(indices.count() == 0) {
		return true;
	} else if(indices.count() == 1) {
		element = parent.ownerDocument().createElement("simple-event");
		
		element.setAttribute("index", indices[0]);
		switch(changes[0].state()) {
			case GivenValue:
				element.setAttribute("given", "true");
				element.setAttribute("value", changes[0].value());
				break;
			case ObviouslyWrong:
			case WrongValue:
			case CorrectValue:
				element.setAttribute("value", changes[0].value());
				break;
			case Marker: {
				QString str;
				
				QBitArray markers = changes[0].markers();
				for(int j = 0; j < markers.size(); ++j) {
					str += markers[j] ? '1' : '0';
				}
				
				element.setAttribute("markers", str);
			} break;
		}
	} else {
		element = parent.ownerDocument().createElement("complex-event");
		for(int i = 0; i < indices.count(); ++i) {
			QDomElement subElement = parent.ownerDocument().createElement("simple-event");
			
			subElement.setAttribute("index", indices[i]);
			switch(changes[i].state()) {
				case GivenValue:
					subElement.setAttribute("given", "true");
					subElement.setAttribute("value", changes[i].value());
					break;
				case ObviouslyWrong:
				case WrongValue:
				case CorrectValue:
					subElement.setAttribute("value", changes[i].value());
					break;
				case Marker: {
					QString str;
					
					QBitArray markers = changes[i].markers();
					for(int j = 0; j < markers.size(); ++j) {
						str += markers[i] ? '1' : '0';
					}
					
					subElement.setAttribute("markers", str);
				} break;
			}
			
			element.appendChild(subElement);
		}
	}
	
	parent.appendChild(element);
	return true;
}

bool Serializer::store(const Game& game, const KUrl& url, QWidget* window) {
	QDomDocument doc( "ksudoku" );
	QDomElement root = doc.createElement( "ksudoku" );
	doc.appendChild( root );	
	
	serializeGame(root, game);
	
	KTemporaryFile file;
	file.open();
	
	QTextStream stream(&file);
	stream << doc.toString();
	stream.flush();
	
	KIO::NetAccess::upload(file.fileName(), url, window);
	return true;
}

}
