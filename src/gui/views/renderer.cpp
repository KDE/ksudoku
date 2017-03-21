/***************************************************************************
 *   Copyright 2008      Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
 *   Copyright 2015      Ian Wadham <iandw.au@gmail.com>                   *
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

#include "renderer.h"

#include <QSvgRenderer>

#include <KDebug>
#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgametheme.h>

#include "settings.h"

#include <QPixmap>
#include <QPainter>

#include <QtDebug>

namespace ksudoku {

Renderer* Renderer::instance() {
	static Renderer instance;
	return &instance;
}
	
Renderer::Renderer() {
	m_renderer = new QSvgRenderer();
	m_cache = new KImageCache(QStringLiteral("ksudoku-cache"), 3*1024);
	m_mathdokuStyle = false;
	
	if(!loadTheme(Settings::theme()))
		kDebug() << "Failed to load any game theme!";
}

Renderer::~Renderer() {
	delete m_cache;
	delete m_renderer;
}

bool Renderer::loadTheme(const QString& themeName) {
	bool discardCache = !m_currentTheme.isEmpty();
	
	if(!m_currentTheme.isEmpty() && m_currentTheme == themeName) {
		kDebug() << "Notice: loading the same theme";
		return true; // this is not an error
	}
	
	m_currentTheme = themeName;
	
	KGameTheme theme;
	if(themeName.isEmpty() || !theme.load(themeName)) {
		kDebug()<< "Failed to load theme" << Settings::theme();
		kDebug() << "Trying to load default";
		if(!theme.loadDefault())
			return false;
		
		discardCache = true;
		m_currentTheme = "default";
	}
	
	bool res = m_renderer->load(theme.graphics());
	kDebug() << "loading" << theme.graphics();
	if(!res)
		return false;
	
	if(discardCache) {
		kDebug() << "discarding cache";
		m_cache->clear();
	}
	
	fillNameHashes();
	return true;
}

void Renderer::fillNameHashes() {
	m_borderNames = QVector<QString>();
	m_borderNames << "";
	m_borderNames << "1";
	m_borderNames << "2";
	m_borderNames << "12";
	m_borderNames << "3";
	m_borderNames << "13";
	m_borderNames << "23";
	m_borderNames << "123";
	m_borderNames << "4";
	m_borderNames << "14";
	m_borderNames << "24";
	m_borderNames << "124";
	m_borderNames << "34";
	m_borderNames << "134";
	m_borderNames << "234";
	m_borderNames << "1234";
	m_borderTypes << QString();
	m_borderTypes << "row";
	m_borderTypes << "column";
	m_borderTypes << "block";
	m_borderTypes << "special";
	m_borderTypes << "block";	// Use block-type borders for cages.
	m_borderTypes << "special";
	m_borderTypes << "special";
	m_borderTypes << QString();
	m_borderTypes << "row_h";
	m_borderTypes << "column_h";
	m_borderTypes << "block_h";
	m_borderTypes << "special_h";
	m_borderTypes << "block_h";	// Use block-type borders for cages.
	m_borderTypes << "special_h";
	m_borderTypes << "special_h";
	m_specialNames << "cell";
	m_specialNames << "cell_preset";
	m_specialNames << "cell";
	m_specialNames << "cell_mistake";
	m_specialNames << "cursor";
	m_specialNames << "valuelist_item";
	m_specialNames << "valuelist_selector";
	m_special3dNames << "cell3d";
	m_special3dNames << "cell3d_preset";
	m_special3dNames << "cell3d";
	m_special3dNames << "cell3d_mistake";
	m_special3dNames << "cursor";
	m_special3dNames << "valuelist_item";
	m_special3dNames << "valuelist_selector";
	// TODO get this hardcoded values from the SVG file
// 	m_markerName << "markers9" << "markers9" //...
}

QPixmap Renderer::renderBackground(const QSize& size) const {
	if(!m_renderer->isValid() || size.isEmpty()) return QPixmap();

	QPixmap pix;
	QString cacheName = QString("background_%1x%2").arg(size.width()).arg(size.height());
	if(!m_cache->findPixmap(cacheName, &pix))
	{
		pix = QPixmap(size);
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		m_renderer->render(&p, "background");
		p.end();
		m_cache->insertPixmap(cacheName, pix);
	}
	return pix;
}

/** Moves a point from its relative position to the base rect (0,0,1,1) to a relative position to rect @p to */
QPointF fromBasetoRect(const QPointF& p, const QRectF& to) {
	return QPointF(p.x()*to.width()+to.left(), p.y()*to.height()+to.top());
}

/** Moves a point from its relative position to rect @p from to a relative position to the base rect (0,0,1,1) */
QPointF fromRectToBase(const QRectF& p, const QRectF& from) {
	return QPointF((p.x()-from.left())/from.width(), (p.y()-from.top())/from.height());
}

/** Moves a point from its relative position to rect @p from to a relative position to rect @p to */
QPointF fromRectToRect(const QPointF& p, const QRectF& from, const QRectF& to) {
	return QPointF((p.x()-from.left())*to.width()/from.width()+to.left(),
	               (p.y()-from.top())*to.height()/from.height()+to.top());
}

QPixmap Renderer::renderSpecial(SpecialType type, int size) const {
	if(!m_renderer->isValid() || size == 0) return QPixmap();
	
	//only show the errors if the option has been set
	if(!Settings::showErrors() && type == SpecialCellMistake ) type = SpecialCell;
	
	QString cacheName = QString("special_%1_%2").arg(m_specialNames[type]).arg(size);
	QPixmap pix;
	if(!m_cache->findPixmap(cacheName, &pix)) {
		pix = QPixmap(size, size);
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		
		// NOTE fix for Qt's QSvgRenderer size reporting bug
		QRectF r(m_renderer->boundsOnElement(m_specialNames[type]));
		QRectF from(r.adjusted(+0.5,+0.5,-0.5,-0.5));
		QRectF to(QRectF(0,0,size,size));
		r.setTopLeft(fromRectToRect(r.topLeft(), from, to));
		r.setBottomRight(fromRectToRect(r.bottomRight(), from, to));
		
		m_renderer->render(&p, m_specialNames[type], r);
		p.end();
		m_cache->insertPixmap(cacheName, pix);
	}

	return pix;
}

QPixmap Renderer::renderSymbol(int symbol, int size, int max, SymbolType type) const {
	if(!m_renderer->isValid() || size == 0) return QPixmap();

	QString set;
	if(max <= 9) {
		set = "symbol";
	} else {
		set = "symbol25";
	}

	QString cacheName = QString("%1_%2_%3_%4").arg(set).arg(symbol).arg(size).arg(type);
	QPixmap pix;
	if(!m_cache->findPixmap(cacheName, &pix)) {
		pix = QPixmap(size, size);
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		
		// NOTE fix for Qt's QSvgRenderer size reporting bug
		QRectF r(m_renderer->boundsOnElement("symbol_1"));
		QRectF from(m_renderer->boundsOnElement("cell_symbol"));
		from.adjust(+0.5,+0.5,-0.5,-0.5); // << this is the fix
		QRectF to(QRectF(0,0,size,size));
		
		r.setTopLeft(fromRectToRect(r.topLeft(), from, to));
		r.setBottomRight(fromRectToRect(r.bottomRight(), from, to));
		
		switch(type) {
			case SymbolPreset:
				if(m_renderer->elementExists(QString("%1_%2_preset").arg(set).arg(symbol))) {
					m_renderer->render(&p, QString("%1_%2_preset").arg(set).arg(symbol), r);
				} else {
					m_renderer->render(&p, QString("%1_%2").arg(set).arg(symbol), r);
				}
				break;
			case SymbolEdited:
				if(m_renderer->elementExists(QString("%1_%2_edited").arg(set).arg(symbol))) {
					m_renderer->render(&p, QString("%1_%2_edited").arg(set).arg(symbol), r);
				} else {
					m_renderer->render(&p, QString("%1_%2").arg(set).arg(symbol), r);
				}
				break;
		}
		p.end();
		m_cache->insertPixmap(cacheName, pix);
	}

	return pix;
}

QPixmap Renderer::renderSymbolOn(QPixmap pixmap, int symbol, int color, int max, SymbolType type) const {
	// We use a smaller size of symbol in Mathdoku and Killer
	// Sudoku, to allow space for the cage labels.
	int size = m_mathdokuStyle ? (pixmap.width()+1)*3/4 : pixmap.width();
	int offset = m_mathdokuStyle ? (pixmap.width()+7)/8 : 0;
	QPixmap symbolPixmap = renderSymbol(symbol, size, max, type);
	if(color) {
		// TODO this does not work, need some other way, maybe hardcode color into NumberType
		QPainter p(&symbolPixmap);
		p.setCompositionMode(QPainter::CompositionMode_Multiply);
		p.setBrush(QBrush(QColor(128,128,128,255)));
		p.drawRect(0, 0, size, size);
		p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
		p.drawPixmap(0, 0, pixmap);
		p.end();
		return symbolPixmap;
	} else {
		QPainter p(&pixmap);
		p.drawPixmap(offset, offset, symbolPixmap);
		p.end();
		return pixmap;
	}
}

QPixmap Renderer::renderMarker(int symbol, int range, int size) const {
	if(!m_renderer->isValid() || size == 0) return QPixmap();
	
	QString set;
	if(range <= 9) {
		set = "symbol";
	} else {
		set = "symbol25";
	}
	
	// TODO this is a hardcoded list of possible marker-groupings
	// replace it with a test for possible markers
	if(range <= 9) {
		range = 9;
	} else if(range <= 16) {
		range = 16;
	} else {
		range = 25;
	}

	QString groupName = QString("markers%1").arg(range);
	QString cacheName = QString("%1_%2_%3").arg(groupName).arg(symbol).arg(size);
	QPixmap pix;
	if(!m_cache->findPixmap(cacheName, &pix)) {
		pix = QPixmap(size, size);
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		
		// NOTE fix for Qt's QSvgRenderer size reporting bug
		QRectF r(m_renderer->boundsOnElement(QString("%1_%2").arg(groupName).arg(symbol)));
		QRectF from(m_renderer->boundsOnElement(QString("cell_%1").arg(groupName)));
		from.adjust(+0.5,+0.5,-0.5,-0.5); // << this is the fix
		QRectF to(QRectF(0,0,size,size));

		r.setTopLeft(fromRectToRect(r.topLeft(), from, to));
		r.setBottomRight(fromRectToRect(r.bottomRight(), from, to));

		m_renderer->render(&p, QString("%1_%2").arg(set).arg(symbol), r);
		p.end();
		m_cache->insertPixmap(cacheName, pix);
	}

	return pix;
}

QPixmap Renderer::renderMarkerOn(QPixmap pixmap, int symbol, int range, int color) const {
	// TODO maybe it would be good to directly integrate the renderMarker implementation and
	// make renderMarker be based on this method. (same for renderSymbol and renderSymbolOn)

	// We use a smaller size of marker in Mathdoku and Killer
	// Sudoku, to allow space for the cage labels.
	int size = m_mathdokuStyle ? (pixmap.width()+1)*3/4 : pixmap.width();
	QPixmap symbolPixmap = renderMarker(symbol, range, size);
	if(color) {
		QPainter p(&symbolPixmap);
		p.setCompositionMode(QPainter::CompositionMode_Multiply);
		p.setBrush(QBrush(QColor(128,128,128,255)));
		p.drawRect(0, 0, size, size);
		p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
		p.drawPixmap(0, 0, pixmap);
		p.end();
		return symbolPixmap;
	} else {
		QPainter p(&pixmap);
		// Offset the marker from 0,0 in Mathdoku and Killer Sudoku.
		int offset = m_mathdokuStyle ? (size + 7)/8 : 0;
		p.drawPixmap(offset, 2*offset, symbolPixmap);
		p.end();
		return pixmap;
	}
}

QPixmap Renderer::renderCageLabelOn(QPixmap pixmap, const QString & cageLabel)
{
	// TODO - Do font setup once during resize? Put 0+-x/ in themes?
	int size = pixmap.width();
	QPainter p(&pixmap);
	p.setPen(QString("white"));	// Text is white on a dark rectangle.
	p.setBrush(Qt::SolidPattern);

	// Cage label uses top 1/4 of pixmap and text is 1/6 height of pixmap.
	QFont f = p.font();
	f.setBold(true);
	f.setPixelSize((size+5)/6);
	p.setFont(f);

	QFontMetrics fm(f);
	int w = fm.width(cageLabel);	// Width of text.
	int h = fm.height();		// Total height of font.
	int a = fm.ascent();		// Height from baseline of font.
	int m = fm.width(QChar('1'))/2;	// Left-right margin = 1/2 width of '1'.

	// Paint background rect: text must be visible in light and dark themes.
	p.fillRect(size/6 - m, (size + 3)/4 - a, w + 2*m, h, Qt::darkGray);
	// Note: Origin of text is on baseline to left of first character.
	p.drawText(size/6, (size+3)/4, cageLabel);

	p.end();
	return pixmap;
}

QPixmap Renderer::renderBorder(int border, GroupTypes type, int size) const {
	if(!m_renderer->isValid() || size == 0) return QPixmap();
	
	QString cacheName = QString("contour_%1_%2_%3").arg(m_borderTypes[type]).arg(m_borderNames[border]).arg(size);
	QPixmap pix;
	if(!m_cache->findPixmap(cacheName, &pix)) {
		pix = QPixmap(size, size);
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		
		// NOTE fix for Qt's QSvgRenderer size reporting bug
		QRectF r(m_renderer->boundsOnElement(QString("%1_%2").arg(m_borderTypes[type]).arg(m_borderNames[border])));
		QRectF from(r.adjusted(+0.5,+0.5,-0.5,-0.5));
		QRectF to(QRectF(0,0,size,size));
		r.setTopLeft(fromRectToRect(r.topLeft(), from, to));
		r.setBottomRight(fromRectToRect(r.bottomRight(), from, to));
		
		m_renderer->render(&p, QString("%1_%2").arg(m_borderTypes[type]).arg(m_borderNames[border]), r);
		p.end();
		m_cache->insertPixmap(cacheName, pix);
	}

	return pix;
}

QPixmap Renderer::renderSpecial3D(SpecialType type, int size) const {
	if(!m_renderer->isValid() || size == 0) return QPixmap();

	QString cacheName = QString("special_%1_%2").arg(m_special3dNames[type]).arg(size);
	QPixmap pix;
	if(!m_cache->findPixmap(cacheName, &pix)) {
		pix = QPixmap(size, size);
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		
		// NOTE fix for Qt's QSvgRenderer size reporting bug
		QRectF r(m_renderer->boundsOnElement(m_special3dNames[type]));
		QRectF from(r.adjusted(+0.5,+0.5,-0.5,-0.5));
		QRectF to(QRectF(0,0,size,size));
		r.setTopLeft(fromRectToRect(r.topLeft(), from, to));
		r.setBottomRight(fromRectToRect(r.bottomRight(), from, to));
		
		m_renderer->render(&p, m_special3dNames[type], r);
		p.end();
		m_cache->insertPixmap(cacheName, pix);
	}

	return pix;
}

}
