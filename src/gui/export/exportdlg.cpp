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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "exportdlg.h"
#include <QSemaphore>

#include "drawfactory.h"
#include "exportpreview.h"
#include "generateevent.h"

#include <knuminput.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kfiledialog.h>
#include <kimageio.h>
#include <kcombobox.h>
#include <ksqueezedtextlabel.h>
//#include <kprogress.h>
#include <kprinter.h>

#include <qcheckbox.h>
#include <qlayout.h>
#include <qstring.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qimage.h>

#include <q3picture.h>
//Added by qt3to4:
#include <QCustomEvent>
#include <QPixmap>
#include <QVBoxLayout>


namespace ksudoku {


ExportDlg::ExportDlg(Puzzle const& currPuzzle, Symbols const& symbols)
	: m_currPuzzle(currPuzzle)    //currPuzzle for display
	, m_puzzleList(*this, &m_currPuzzle) //currPuzzle for use as template
	, m_currSymbols(symbols)
{
	m_drawer = DrawFactory().create_instance(m_currPuzzle, m_currSymbols);
	if(!m_drawer){
		KMessageBox::information(0, i18n("Sorry. I am not able to export this puzzle type (yet)"));
		done(QDialog::Rejected); //return rejected
	}

	//completion dialog
	ExportPreview* m_qwPreview = new ExportPreview(this,fPreview);
	QLayout* ql = fPreview->layout();
	if(! ql)
		ql = new QVBoxLayout(fPreview);
	ql->add(m_qwPreview);

	//fill paper options
	kcbPageSize->insertStringList(m_pageSize.pageSizeNames());

	///@TODO implement multiple output pages from same settings (feature)
	kispPageCount->hide(); //hide for now
	kSqueezedTextLabel2_2->hide(); // ""
	

	//connect ExportDlgBase signals
	//   connect buttons
//	connect(kpbHelp    , SIGNAL(released()         )
//	       ,this       , SLOT  (help    ()         ) );
	connect(kpbCancel  , SIGNAL(released()         )
	       ,this       , SLOT  (cancel  ()         ) );
	connect(kpbPrint   , SIGNAL(released()         )
	       ,this       , SLOT  (print   ()         ) );
	connect(kpbExport  , SIGNAL(released()         )
	       ,this       , SLOT  (save    ()         ) );
	//   connect others
	connect(this       , SIGNAL(aValueChanged()    )
	       ,this       , SLOT  (updatePreview()    ) );
	//connect ExportDlg signals
	connect(this       , SIGNAL(updatePreviewSig() )
	       ,m_qwPreview, SLOT  (draw            () ) );

	connect(kpbRegenerate, SIGNAL(clicked        ())
	       ,this         , SLOT  (reCreatePuzzles()) );

	// connect output size settings
	connect(kcbPageSize, SIGNAL(activated    (const QString&) )
	       ,this       , SLOT  (setOutputType(const QString&) ) );
	connect(kisbHres   , SIGNAL(valueChanged   (int) )
	       ,this       , SLOT  (setOutputWidth (int) ) );
	connect(kisbVres   , SIGNAL(valueChanged   (int) )
	       ,this       , SLOT  (setOutputHeight(int) ) );
	connect(cbLockCustomSize, SIGNAL(stateChanged       (int) )
	       ,this            , SLOT  (pageSizeLockChanged(int) ) );
}

ExportDlg::~ExportDlg()
{
	///@TODO store settings permanently (if accepted)
	delete m_drawer;
}

void ExportDlg::polish()
{
	setSettings(); //restore saved settings
	m_puzzleList.resize(1); //there is at least 1 puzzle visable
	if(m_expDlgSettings.generatePreviewGame())
		createPuzzles();
	updatePreview();
	updateProgressBar();
}

void ExportDlg::customEvent(QCustomEvent* e)
{
	if(e->type() == GENERATE_EVENT){  // It must be a GenerateEvent
		GenerateEvent* ge = dynamic_cast<GenerateEvent*>(e);
		switch (ge->event()){
			case ksudoku::puzzleChanged:
				updateProgressBar();
				emit updatePreviewSig();
			break;
			case ksudoku::sizeChanged:
				updateProgressBar();
			break;
			//default:
			//	;//do nothing
		}
	}
}

void ExportDlg::updatePreview()
{
	getSettings(); //probably settings changed, store them (!= write to disk)

	//only create puzzles here if preview needs to display them
	if(m_expDlgSettings.generatePreviewGame())
		createPuzzles();//m_puzzleList.generate();

	//m_qwPreview->draw();
	emit updatePreviewSig(); ///@todo figure out why calling draw() directly makes it crash
}

void ExportDlg::setSettings()
{
	//game count
	kisbHCount->setValue(m_expDlgSettings.gameHCount());
	kisbVCount->setValue(m_expDlgSettings.gameVCount());
	//spacing
	kisbMargin->setValue(m_expDlgSettings.gameMargin());
	kdsbAspectRatio->setValue(m_expDlgSettings.gameAspectRatio());
	
	if(m_pageSize.equal( m_expDlgSettings.pageSize()
	                   , QSize(m_expDlgSettings.pSizeHRes(), m_expDlgSettings.pSizeVRes())
	                   ) )
		setOutputType(m_expDlgSettings.pageSize());
	else
		setOutputSize(  QString::null
		              , m_expDlgSettings.pSizeVRes()
		              , m_expDlgSettings.pSizeHRes());
	cbLockCustomSize->setChecked(m_expDlgSettings.pSizeLock());

	//for view
	cbGenPrevGame->setChecked(m_expDlgSettings.generatePreviewGame());
	cbUseCurrGame->setChecked(m_expDlgSettings.useCurrentGame());
}

void ExportDlg::getSettings()
{
	//game count
	m_expDlgSettings.setGameHCount(kisbHCount->value());
	m_expDlgSettings.setGameVCount(kisbVCount->value());
	//spacing
	m_expDlgSettings.setGameMargin(kisbMargin->value());
	m_expDlgSettings.setGameAspectRatio(kdsbAspectRatio->value());
	//output size
	m_expDlgSettings.setPageSize( kcbPageSize->currentText());
	m_expDlgSettings.setPSizeHRes(kisbHres->value());
	m_expDlgSettings.setPSizeVRes(kisbVres->value());
	m_expDlgSettings.setPSizeLock(cbLockCustomSize->isChecked());

	//for view
	m_expDlgSettings.setGeneratePreviewGame(cbGenPrevGame->isChecked());
	m_expDlgSettings.setUseCurrentGame(cbUseCurrGame->isChecked());
}

QSize ExportDlg::currentPageSize() const
{
	return QSize(kisbHres->value(),kisbVres->value());
}

void ExportDlg::setAspectRatio()
{
	mPSizeAspect = static_cast<float>(kisbHres->value()) / kisbVres->value();
}

void ExportDlg::print(){
	KPrinter printer;
	printer.removeStandardPage(1); //there is only 1 standard page

	if (printer.setup())
	{

		QPainter p;
		p.begin(&printer);
		draw(p, printer->height(), printer->width());
		p.end();
	}
}

void ExportDlg::save(){
	//get filename for saving
	KImageIO::registerFormats(); ///@TODO only need to do this once ???, move to main

	QString filename;
	QString mimeType;
	bool noFilename = true;
	while(noFilename){
		filename = KFileDialog::getSaveFileName( QString::null
		           , KImageIO::pattern(KImageIO::Writing),0,i18n("Export Ksudoku"));
		if(filename.isNull())
			return; //canceled

		//check if filename is valed etc.
		mimeType = KImageIO::type(filename);
		if( ! mimeType ){
			KMessageBox::information(this, i18n("Sorry. I am not able to export to this filetype (filetype is guessed from filename suffix).\nHint: select a type from the filter bar instead"));
			continue;
		}
		else{
			if(QFile::exists(filename))
				if(KMessageBox::Yes != KMessageBox::questionYesNo(this, i18n("A document with this name already exists.\nDo you want to overwrite it?")))
					continue;
			noFilename = false; //filename given and correct
		}
	}

	//create the data to export
	QSize size = currentPageSize();
	///@todo fixme, make resolution user configurable
	double res = 92/25.4; //92pi (1 inch == 25.4 mm), (dpc would be si)
	int w = static_cast<int>(size.width() * res);
	int h = static_cast<int>(size.height()* res);

	//make sure puzzles exists !!
	createPuzzles();
	while(m_puzzleList.running()) //consider running as puzzles not available
		usleep(50000);

	QPixmap pm(w,h);
	QPainter p(&pm);

	draw(p, h, w, true, cbUseCurrGame->isChecked());
	p.end();

	//write the data to file
	pm.save(filename, mimeType, -1);
}


void ExportDlg::cancel()
{
	///@todo check if all changes are restored (not saved)
	done(QDialog::Rejected); //return rejected
}

void ExportDlg::updateProgressBar()
{
	uint currCount = m_puzzleList.count();
	uint currSize  = m_puzzleList.size ();
	QString status;
	if( ( ! cbGenPrevGame->isChecked()) ||(currCount == currSize)){
		status = i18n("1 puzzle available","%n puzzles available",currCount);
		//set currCount to 0 so progress bar is empty
		currCount = 0;
	}
	else{
		status = i18n("generating puzzle %1 of %2", currCount,currSize);
	}
	kProgress->setTotalSteps(currSize);
	kProgress->setFormat(status);
	kProgress->setProgress(currCount);
}

void ExportDlg::draw(QPainter& qpainter, int height, int width) const
{
	bool drawContent = m_expDlgSettings.generatePreviewGame();
	bool useCurrent  = cbUseCurrGame->isChecked();

	draw(qpainter, height, width, drawContent, useCurrent);
}


void ExportDlg::draw(QPainter& qpainter, int height, int width
                     , bool drawContent, bool useCurrent) const
{
	qpainter.fillRect(0,0,width,height,QColor("white"));  //draw target (paper)

	if( ! m_drawer)
		return; ///@todo maybe write msg that no drawer is set, or consider this a bug??

	int vcount = m_expDlgSettings.gameVCount();
	int hcount = m_expDlgSettings.gameHCount();
	int margin = m_expDlgSettings.gameMargin();
	int w      = (width  - margin*(hcount-1)) / hcount;
	int h      = (height - margin*(vcount-1)) / vcount;
	int wp     = w;
	int hp     = h;

	float viewAspR  = static_cast<float>(w) / h;
	float gameAspR  = m_expDlgSettings.gameAspectRatio();

	if( gameAspR > viewAspR)
		hp = static_cast<int>(w / gameAspR);
	else
		wp = static_cast<int>(h * gameAspR);


	for(int x=0; x < vcount; ++x){
		for(int y=0; y < hcount; ++y){
			qpainter.translate(y*(w+margin),x*(h+margin));
			uint index = x+(vcount)*y;

			//always draw raster
			m_drawer->drawRaster(qpainter, hp, wp);
			//only view content if requested
			if(drawContent){
				if(index == 0 && useCurrent) //only if user wants current game
					m_drawer->drawValues(qpainter, hp, wp);
				else{
					Puzzle const* puzzle = (index < m_puzzleList.size()) ? m_puzzleList[index] : 0;
					if(puzzle){
						DrawBase* drawer = DrawFactory().create_instance(*puzzle, m_currSymbols);
						drawer->drawValues(qpainter, hp, wp);
						delete drawer;
					}
				}
			}
			qpainter.resetXForm();
		}
	}
}

void ExportDlg::createPuzzles()
{
	uint puzzleCount = m_expDlgSettings.gameVCount() * m_expDlgSettings.gameHCount();

	//only create games if there are less games than puzzleCount
	uint currCount = m_puzzleList.count();
	if(puzzleCount <= currCount)
		return;

	if(m_puzzleList.size() < puzzleCount)
		m_puzzleList.resize(puzzleCount);

	m_puzzleList.generate();
}

void ExportDlg::reCreatePuzzles()
{
	m_puzzleList.resize(m_expDlgSettings.gameVCount() * m_expDlgSettings.gameHCount());
	m_puzzleList.regenerate();
}

void ExportDlg::setOutputSize(const QString& type, int height, int width)
{
	//prevent changes made here to cause an emit
	kisbVres->blockSignals(true);
	kisbHres->blockSignals(true);

	//no warning is given if nothing is changed, fix this??
	if(type.isEmpty()){
		if(height > 0){
			if(m_expDlgSettings.pSizeLock())
				kisbHres->setValue(static_cast<int>(height * mPSizeAspect));
			kisbVres->setValue(height);
		}
		else if(width  > 0){
			if(m_expDlgSettings.pSizeLock())
				kisbVres->setValue(static_cast<int>(width / mPSizeAspect));
			kisbHres->setValue(width);
		}
		//else
				//return error

		//set view to custom (expect it to be last entry)
		kcbPageSize->setCurrentItem(kcbPageSize->count()-1);
	}
	else{
		kcbPageSize->setCurrentItem(m_pageSize.index(type));
		QSize size(m_pageSize.size(type)); ///@todo check what happens it type doesn't exist
		kisbVres->setValue(size.height());
		kisbHres->setValue(size.width ());
	}
	
	kisbVres->blockSignals(false);
	kisbHres->blockSignals(false);

	updatePreview();
}

}

#include "exportdlg.moc"

