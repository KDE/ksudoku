//
// C++ Interface: printdialogpage
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KSUDOKUPRINTDIALOGPAGE_H
#define KSUDOKUPRINTDIALOGPAGE_H

//generated:
#include "ui_printdialogpagedlg.h"

#include <qspinbox.h>
#include <kdeprint/kprintdialogpage.h>
#include <knuminput.h>

namespace ksudoku {

class PrintPreview;
class Print;
class DrawBase;

#define SCALE     "kde-ksudoku-scale"
#define ASPECT    "kde-ksudoku-DesiredAspectRatio"

/**
	Add Ksudoku options-tab to KPrinter dialog
	
	@TODO when print options change (paper orientation for instance)
	      this should reflect in the preview (but it this info is only
	      available after the fat lady sings)

*/
class PrintDialogPage : public KPrintDialogPage
{
public:
	PrintDialogPage(Print const& print, QWidget *parent = 0, const char *name = 0 );
	~PrintDialogPage();
	
	///reimplemented from KPrintDialogPage
	void getOptions( QMap<QString,QString>& opts, bool incldef = false );
	///reimplemented from KPrintDialogPage
	void setOptions( const QMap<QString,QString>& opts );
	///reimplemented from KPrintDialogPage
	bool isValid( QString& msg );

	//getters
	int   scale () const { return m_dlg.sbScale   ->value(); }
	float aspect() const { return m_dlg.kdspAspect->value(); }

private:
	///ksudoku print options tab, added to kprinter
	Ui_PrintDialogPageDLG m_dlg;
};

}

#endif
