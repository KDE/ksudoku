//
// C++ Interface: generateevent
//
// Description: 
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KSUDOKUGENERATEEVENT_H
#define KSUDOKUGENERATEEVENT_H

#include <qevent.h>
//Added by qt3to4:
#include <QCustomEvent>
#include "ksudoku_types.h"

namespace ksudoku {

enum GEType { puzzleChanged, sizeChanged };

/**
 * Class "needed" for communication between Qt threads
 */
class GenerateEvent : public QCustomEvent{
public:
	///constructor
	inline GenerateEvent(GEType event);

	///@return event type
	inline GEType event() const;
private:
	GEType m_event;
};

GenerateEvent::GenerateEvent(GEType event)
	: QCustomEvent( GENERATE_EVENT )
	, m_event(event)
{};
	
GEType GenerateEvent::event() const
{
	return m_event;
}

}

#endif
