#include "serializer.h"
//#include "sksolver.h"
#include "ksudokugame.h"
#include "puzzle.h"

#include <qdom.h>
//Added by qt3to4:
#include <QList>
#include <kurl.h>
#include <ktemporaryfile.h>
#include <kio/netaccess.h>
#include <qfile.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "ksudoku.h"

namespace ksudoku {

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
	bool hasSolver = false;
	SKSolver* solver = 0;
	bool hasValues = false;
	QString valuesStr;
	bool hasSolution = false;
	QString solutionStr;
	
	QString content;
	QDomNode child = element.firstChild();
	while (!child.isNull()) {
		if(child.isElement()) {
			if(child.nodeName() == "graph") {
				if(hasSolver) {
					delete solver;
					return 0;
				}
					
				solver = deserializeGraph(child.toElement());
				hasSolver = true;
			} else if(child.nodeName() == "values") {
				if(hasValues) {
					delete solver;
					return 0;
				}
				
				valuesStr = child.toElement().text();
				hasValues = true;
			} else if(child.nodeName() == "solution") {
				if(hasSolution) {
					delete solver;
					return 0;
				}
				
				solutionStr = child.toElement().text();
				hasSolution = true;
			}
		}
		child = child.nextSibling();
	}
	
	if(!solver) return 0;
	if(valuesStr.length() != solver->g->size) {
		delete solver;
		return 0;
	}
	if(solutionStr.length() != 0 && solutionStr.length() != solver->g->size) {
		delete solver;
		return 0;
	}
	
	Puzzle* puzzle = new Puzzle(solver, hasSolution);
	
	QByteArray values;
	values.resize(solver->g->size);
	for(int i = 0; i < solver->g->size; ++i) {
		if(valuesStr[i] == '_')
			values[i] = 0;
		else
			values[i] = (valuesStr[i].toAscii()) - 'a';
	}
	
	QByteArray solution;
	if(solutionStr.length() != 0) {
		solution.resize(solver->g->size);
		for(int i = 0; i < solver->g->size; ++i) {
			if(solutionStr[i] == '_')
				solution[i] = 0;
			else
				solution[i] = (valuesStr[i].toAscii()) - 'a';
		}
	}
	
	puzzle->init(values, solution);
	return puzzle;
}

static int readInt(QDomElement element, QString name, int* err)
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

SKSolver* Serializer::deserializeGraph(QDomElement element) {
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
	if(type == "roxdoku")
		d3 = true;
	else if(type == "custom")
	{
		int err=0;
		int ncliques = readInt(element,"ncliques", &err);
		int sizeX = readInt(element,"sizeX",&err);
		int sizeY = readInt(element,"sizeY",&err);
		int sizeZ = readInt(element,"sizeZ",&err);
		//int size = sizeX*sizeY*sizeZ;

		QString name = element.attribute("name");;

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
				cliques += sz + " " + child.toElement().text();
			}
			child = child.nextSibling();
		}
		
		GraphCustom* gc = new GraphCustom();
		gc->init(name.toLatin1(), order, sizeX, sizeY, sizeZ, ncliques, cliques.toLatin1());
		if(gc->valid==false) return 0;
		
		SKSolver* solver = new SKSolver(gc);
		solver->setType(custom);
		return solver;
	}
	

	SKSolver* solver = new SKSolver(order, d3);
	solver->init();
	return solver;
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

SKSolver* Serializer::loadCustomShape(const KUrl& url, QWidget* window, QString *errorMsg) {
	if ( url.isEmpty() ) return 0;
	QString tmpFile;
	bool success = false;
	QDomDocument doc;
	if(KIO::NetAccess::download(url, tmpFile, window) ) {
		QFile file(tmpFile);
		if(file.open(QIODevice::ReadOnly)) {
			int errorLine;
			if(!doc.setContent(&file, 0, &errorLine)) {
				printf("Error on %d\n", errorLine);
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
			printf("Error on\n");
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

	SKSolver* sk = (SKSolver*) game.puzzle()->solver();
	QString name    =  ((GraphCustom*)sk->g)->name;
	KSudoku* p = (KSudoku*) window;
	if(!p->shapes().contains(name))
	{
		KStandardDirs myStdDir;
		const QString destDir = myStdDir.saveLocation("data", /* TODO PORT kapp->instanceName() +*/ "ksudoku/", true);
		KStandardDirs::makeDir(destDir);

		QString path = destDir + name + ".xml";
		KUrl url;
		url.setPath(path);

		Serializer::storeCustomShape( sk, url ,window );
		p->updateCustomShapesList();
		
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
	serializeGraph(element, puzzle->solver());
	
	for(int i = 0; i < puzzle->size(); ++i) {
		if(puzzle->value(i) == 0)
			contentStr += '_';
		else
			contentStr += 'a' + puzzle->value(i);
	}
	
	QDomElement content = doc.createElement("values");
	content.appendChild(doc.createTextNode(contentStr));
	element.appendChild(content);
	
	if(puzzle->hasSolution()) {
		contentStr = QString();
		for(int i = 0; i < puzzle->size(); ++i) {
			if(puzzle->solution(i) == 0)
				contentStr += '_';
			else
				contentStr += 'a' + puzzle->solution(i); //->numbers[i]);
		}
		content = doc.createElement("solution");
		content.appendChild(doc.createTextNode(contentStr));
		element.appendChild(content);
	}
	
	parent.appendChild(element);
	return true;
}

bool Serializer::serializeGraph(QDomElement& parent, const SKSolver* puzzle) {
	QDomElement element = parent.ownerDocument().createElement("graph");
	element.setAttribute("order", puzzle->order);
	//element.setAttribute("size", puzzle->size());
	
	GameType type = puzzle->type();
	element.setAttribute("type" , (type == sudoku) ? "sudoku" : (type == roxdoku) ? "roxdoku" : "custom");
	if(type == custom)
	{
		GraphCustom* g = (GraphCustom*) puzzle->g;
		element.setAttribute("ncliques", (int) g->cliques.size());
		element.setAttribute("name", g->name);
		element.setAttribute("sizeX", g->sizeX());
		element.setAttribute("sizeY", g->sizeY());
		element.setAttribute("sizeZ", g->sizeZ());

		for(int i=0; i<g->cliques.size(); i++)
		{
			QDomElement clique = parent.ownerDocument().createElement("clique");
			clique.setAttribute("size",  (int) g->cliques[i].size());
			//serialize clique
			QString contentStr = "";
			for(int j=0; j<g->cliques[i].size(); j++)
			{
				contentStr += QString::number(g->cliques[i][j]) + " ";
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

bool Serializer::store(const Game& game, const KUrl& /*url*/, QWidget* /*window*/) {
	QDomDocument doc( "ksudoku" );
	QDomElement root = doc.createElement( "ksudoku" );
	doc.appendChild( root );	
	
	serializeGame(root, game);
	
	KTemporaryFile tmp;
	//(*tmp.textStream()) << doc.toString(); //TODO PORT
	tmp.close();
	//KIO::NetAccess::upload(tmp.name(), url, window); //TODO PORT
	//tmp.unlink(); //TODO PORT
	return true;
}

bool Serializer::storeCustomShape(const SKSolver* solver, const KUrl& /*url*/, QWidget* /*window*/) {
	QDomDocument doc( "ksudoku-graph" );
	QDomElement root = doc.createElement( "ksudoku-graph" );
	doc.appendChild( root );	
	
	serializeGraph(root, solver);
	
	KTemporaryFile tmp;
	//(*tmp.textStream()) << doc.toString(); //TODO PORT
	tmp.close();
	//KIO::NetAccess::upload(tmp.name(), url, window); //TODO PORT
	//tmp.unlink(); //TODO PORT
	return true;
}

}
