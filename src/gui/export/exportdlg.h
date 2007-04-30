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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef KSUDOKUEXPORTDLG_H
#define KSUDOKUEXPORTDLG_H

#include "exportdlgsettings.h"

#include "pagesize.h"
#include "exportpuzzles.h"

#include <qobject.h>
//Added by qt3to4:
#include <QCustomEvent>

//generated
#include "ui_exportdlgbase.h"

namespace ksudoku {

class Puzzle;
class DrawBase;
class ExportPreview;
class Symbols;

/**
 * Export functions (and dialog)
 * @todo create better dialog
 */
class ExportDlg : public ExportDlgBase
{
	Q_OBJECT
public:
	ExportDlg(Puzzle const& currGame, Symbols const& symbols);
	virtual ~ExportDlg();

	///Manage Output size
	///If @arg type is given (not empty), @arg height en @arg width are ignored
	///If @arg height and or @arg width are given, @arg type is set to Custum
	///If @arg height or @arg width is less than 1, its value is ignored
	///@warning @arg type is not checked for validity => make sure it makes sense
	void setOutputSize(const QString& type, int height, int width);

public slots:
	///reimplemented from qwidget
	virtual void polish();

	void updatePreview();

	///draw contents to qpainter honoring ExportDlgSettings
	void draw(QPainter& qpainter, int height, int width) const;
	///draw contents to qpainter using specific settings
	void draw(QPainter& qpainter, int height, int width
	          , bool drawContent, bool useCurrent) const;

	///leave dialog, not storing changes
	void cancel();
	///export contents honoring ExportDlgSettings to printer
	void print();
	///export contents honoring ExportDlgSettings to file
	void save();

	///@return page size selected by user (or set by loading settings)
	QSize currentPageSize() const;

	///Change output type (and set height and width values in view)
	inline void setOutputType(const QString& type);
	///Change output height (and changes type to custom in view)
	inline void setOutputHeight(int height);
	///Change output width (and changes type to custom in view)
	inline void setOutputWidth(int width);

signals:
	void updatePreviewSig();

protected:
	///reimplemented from QObject
	virtual void customEvent(QCustomEvent* e);
	
	///get visable and unvisavle values and store them
	void getSettings();
	///set visable and unvisavle values from stored data
	void setSettings();

private slots:
	inline void pageSizeLockChanged(int) { setAspectRatio(); }

	///update the progressbar, thread safe
	void updateProgressBar();

	///create puzzles to export
	void createPuzzles();
	///recreate puzzles to export (aka regenerate)
	///(first resizes m_puzzleList to current size)
	void reCreatePuzzles();

private:
	///set the aspectratio (used by setOutputSize) to reflect current
	///width and height
	void setAspectRatio();


	///current puzzle export is called from
	Puzzle const& m_currPuzzle;
	///generates and holds puzzles for export
	ExportPuzzles m_puzzleList;
	///symbols used for current puzzle
	Symbols const& m_currSymbols;

	///settings used for export
	Ui_ExportDlgSettings m_expDlgSettings;

	DrawBase* m_drawer;
	ExportPreview*  m_qwPreview;

	
	PageSize m_pageSize;

	///aspect size used by setOutputSize if size is locked
	float mPSizeAspect;
};


void ExportDlg::setOutputType(const QString& type)
{
	setOutputSize(type,-1,-1);
}

void ExportDlg::setOutputHeight(int height)
{
	setOutputSize(QString(), height, -1);
}

void ExportDlg::setOutputWidth(int width)
{
	setOutputSize(QString(), -1, width);
}


}

#endif
