//
// C++ Implementation: exportsettings
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "exportdlgsettings.h"

#include <kapplication.h>


namespace ksudoku {

ExportDlgSettings::ExportDlgSettings()
	: m_kconfig(*KApplication::kApplication()->sessionConfig())
{
}


ExportDlgSettings::~ExportDlgSettings()
{
}


}
