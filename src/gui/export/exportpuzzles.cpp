//
// C++ Implementation: exportpuzzles
//
// Description: 
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "exportpuzzles.h"

#include "puzzlefactory.h"
#include "generateevent.h"

#include <kapplication.h>

#define MAX_OTHERACTIONS  1
//#define MAX_DELETION_LOCK INT_MAX

namespace ksudoku {

ExportPuzzles::ExportPuzzles(QObject& eventReceiver, Puzzle const* currPuzzle)
	: QThread() ///@BUG (in qthread ?) if priority is set, this thread doesn't do a thing
	, m_currPuzzle(currPuzzle)
	, m_size(0)
	, m_otherActionRequired(MAX_OTHERACTIONS)
	, m_eventReceiver(eventReceiver)
{
}


ExportPuzzles::~ExportPuzzles()
{
	destroy();
}

void ExportPuzzles::destroy()
{
	while( ! m_puzzleList.empty()){
		Puzzle* p = m_puzzleList.last();
		delete p;
		p = 0;
		m_puzzleList.pop_back();
	}
}

void ExportPuzzles::run()
{
	QMutexLocker locker(&m_mutex);

	uint currCount = count();
	uint currSize  = size();

	for(uint i=currCount; i < currSize; ++i){
		if(MAX_OTHERACTIONS - m_otherActionRequired.available())
			break;
		int order      = m_currPuzzle->order     ();
		int difficulty = m_currPuzzle->difficulty();
		int symmetry   = m_currPuzzle->symmetry  ();
		m_puzzleList.push_back(PuzzleFactory().create_instance(sudoku, order, difficulty, symmetry, false));

		GenerateEvent* ge = new GenerateEvent(ksudoku::puzzleChanged);
		KApplication::postEvent( &m_eventReceiver , ge );  // Qt will delete it when done
	}
	GenerateEvent* ge = new GenerateEvent(ksudoku::puzzleChanged);
	KApplication::postEvent( &m_eventReceiver , ge );  // Qt will delete it when done;
		//above event shouldn't be there, but cant hurt, can it?
		//atleast it solves the missing update when
		//1 puzzle has to be generated at when saving (and
		//no puzzle was generated before)
}

void ExportPuzzles::resize(uint newSize)
{
	m_otherActionRequired++;

	while(running())
		msleep(50);

	QMutexLocker locker(&m_mutex);
	m_otherActionRequired--; //got lock => release interupt
	
	m_size = newSize;
	m_deletionLock.lock();
	while(count() > size())
		m_puzzleList.pop_back();
	m_deletionLock.unlock();
		
	GenerateEvent* ge = new GenerateEvent(ksudoku::sizeChanged);
	KApplication::postEvent( &m_eventReceiver , ge );  // Qt will delete it when done
}


void ExportPuzzles::regenerate()
{
	m_otherActionRequired++;

	while(running())
		msleep(50);

	QMutexLocker locker(&m_mutex);
	m_otherActionRequired--; //got lock => release interupt

	m_deletionLock.lock();
	destroy();
	m_deletionLock.unlock();

	//should unlock here ??
	generate();
}


}


#include "exportpuzzles.moc"
