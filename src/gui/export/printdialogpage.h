/***************************************************************************
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
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

#ifndef KSUDOKUPRINTDIALOGPAGE_H
#define KSUDOKUPRINTDIALOGPAGE_H

//generated:
#include "ui_printdialogpagedlg.h"

#include <qspinbox.h>
#include <knuminput.h>

namespace ksudoku {

class Print;

/**
	Add Ksudoku options-tab to KPrinter dialog
	
	@TODO when print options change (paper orientation for instance)
	      this should reflect in the preview (but it this info is only
	      available after the fat lady sings)

*/
class PrintDialogPage : public QWidget
{
public:
	explicit PrintDialogPage(Print const& print, QWidget *parent = 0, const char *name = 0 );
	~PrintDialogPage();
	
	///reimplemented from KPrintDialogPage
	bool isValid( QString& msg );

	//getters
	int   scale () const { return m_dlg.sbScale   ->value(); }
	void  setScale( int printScale ) { m_dlg.sbScale   ->setValue( printScale); }
	float aspect() const { return m_dlg.kdspAspect->value(); }
	void  setAspect( float printAspect ) { m_dlg.kdspAspect->setValue( printAspect ); }

private:
	///ksudoku print options tab, added to qprinter
	Ui_PrintDialogPageDLG m_dlg;
};

}

#endif
