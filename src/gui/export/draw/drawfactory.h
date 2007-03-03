//
// C++ Interface: drawfactory
//

// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef KSUDOKUDRAWFACTORY_H
#define KSUDOKUDRAWFACTORY_H

#include "drawbase.h"

namespace ksudoku {


/**
 * Generate Draw instances
 */
class DrawFactory{
public:
	DrawFactory();
	~DrawFactory();

	///@return  Draw instance for puzzle, or 0 on error
	///@warning caller is responsible for deletion
	DrawBase* create_instance(Puzzle const& puzzle, Symbols const& symbols) const;
};

}

#endif
