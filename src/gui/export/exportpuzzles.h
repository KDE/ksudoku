//
// C++ Interface: exportpuzzles
//
// Description: 
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KSUDOKUEXPORTPUZZLES_H
#define KSUDOKUEXPORTPUZZLES_H

#include <qthread.h>
#include <qobject.h>
#include <q3valuelist.h>
#include <qmutex.h>
#include <q3semaphore.h>

namespace ksudoku {

class Puzzle;

typedef Q3ValueList<Puzzle*> ListType;

/**
 * Holds the games generated for export
 */
class ExportPuzzles : public QObject, public QThread
{
	Q_OBJECT
private:
	///prevent copy constructor (not implemented)
	ExportPuzzles(ExportPuzzles const& other);
	///prevent assignment (not implemented)
	ExportPuzzles& operator=(ExportPuzzles const& other);
public:
	///Constructor. @arg currPuzzle is as template for (re)generating puzzles
	ExportPuzzles(QObject& eventReceiver, Puzzle const* currPuzzle);
	~ExportPuzzles();

	//getters
	uint size () const  { return m_size              ; }
	uint count() const  { return m_puzzleList.count(); }

	///resize puzzle capacity
	void resize(uint const newSize);

//	Puzzle*       operator[](uint const index)       { return m_puzzleList[index]; }
	///overloaded index operator
	Puzzle const* operator[](uint const index) const { return (index < count()) ? m_puzzleList[index] : 0; }

	///prevent deletion of puzzles  in m_puzzleList
	void lockDeletion()   { m_deletionLock.lock(); }
	///remove puzzle deletion lock (@see lockDeletion);
	///@return whatever QSemaphore::operator returns
	void unlockDeletion() { m_deletionLock.unlock(); }

signals:
	///inform which puzzle index has changed
	void puzzleChanged(int);
	///inform that a puzzle changed
	void aPuzzleChanged();
	///inform when the size changes
	void sizeChanged();

public slots:
	///generate data for puzzles. (Only generates data if
	/// size > count)
	void generate() { start(); }
	
	///Regenerate data for puzzles
	void regenerate();

protected:
	///Reimplemented from QThread
	///performs the real generation of Puzzles
	virtual void run();
	
private:
	///unconditionally delete all puzzles in m_puzzleList
	void destroy();

	ListType m_puzzleList;
	///pointer to external puzzle. Is used as basis for all puzzles
	Puzzle const* m_currPuzzle;

	///max puzzles stored in m_puzzleList
	int m_size;

	///used for interupting run (generate). This is used in stead of
	///QThread::terminate() (which is deprecated)
	///using qsemaphore and not an int just for safety
	QSemaphore m_otherActionRequired;

	///mutex used by @ref run
	QMutex m_mutex;
	///QMutex used by controling deletion of data
	QMutex m_deletionLock;

	///the object where the events should go (qt 3.3 can't handle threaded signals/slots)
	QObject& m_eventReceiver;
};

}

#endif
