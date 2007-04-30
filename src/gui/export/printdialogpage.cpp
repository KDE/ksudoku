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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "printdialogpage.h"
#include "print.h"

#include <klocale.h>

#include <qlayout.h>
#include <qstring.h>
#include <QVBoxLayout>
#include <QPaintEvent>


namespace ksudoku {

//start PrintPreview Class
///@todo move PrintPreview class to its own file
class PrintPreview : public QWidget
{
public:
	PrintPreview(Print const& print, PrintDialogPage const& pdp, QWidget *parent)
	: QWidget(parent)
	, m_print(print)
	, m_pdp(pdp)
	{
		setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding); //not realy needed
	}

protected:
	void paintEvent (QPaintEvent* e){
		float scale  = m_pdp.scale() / 100.0 ;
		float aspect = m_pdp.aspect();

		m_qpainter.begin(this);
		m_qpainter.fillRect(0,0,width(),height(),QColor("white"));            //draw target (paper)
		m_print.drawUsingPrinterSettings(m_qpainter, scale, aspect
		                                , height(), width()); //draw game frames
		m_qpainter.end();

		QWidget::paintEvent(e);
	}

private:
	///external class used to generate preview
	Print const& m_print;
	///external class used to generate preview
	PrintDialogPage const& m_pdp;

	///recycling the QPainter
	QPainter m_qpainter;
};
//end PrintPreview Class


PrintDialogPage::PrintDialogPage(Print const& print, QWidget *parent, const char *name)
	: KPrintDialogPage() //TODO PORT
	, m_dlg()
{
	setTitle( i18n("KSudoku options"));

	//completion PrintDialogPageDLG
	PrintPreview* pp = new PrintPreview(print, *this, m_dlg.previewFrame);
	QLayout* ql = m_dlg.previewFrame->layout();

	//TODO PORT
	/*ql->add(pp);
	ql = new QVBoxLayout(this);
	ql->add(&m_dlg);
	
	connect(&m_dlg, SIGNAL(aValueChanged()) ,
	        pp    , SLOT  (update       ()) );*/
}

PrintDialogPage::~PrintDialogPage()
{
}


void PrintDialogPage::getOptions( QMap<QString,QString>& opts, bool /*incldef*/ )
{
	opts[ SCALE     ] = QString::number(scale () );
	opts[ ASPECT    ] = QString::number(aspect() );
}

void PrintDialogPage::setOptions( const QMap<QString,QString>& opts )
{
	m_dlg.sbScale   ->setValue((opts[ SCALE     ]).toInt  ());
	m_dlg.kdspAspect->setValue((opts[ ASPECT    ]).toFloat());
}

bool PrintDialogPage::isValid( QString& /*msg*/)
{
	//msg not used, m_dlg checked by ui

	return true;
}

}
