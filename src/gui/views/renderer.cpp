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
#include "ksudoku_logging.h"

#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

#include <KGameTheme>
#include <KGameThemeProvider>

#include "settings.h"

namespace ksudoku {

Renderer* Renderer::instance() {
	static Renderer instance;
	return &instance;
}
	
Renderer::Renderer() {
	m_themeProvider = new KGameThemeProvider(QByteArray()); // empty config key to disable internal config storage
	m_renderer = new QSvgRenderer();
	m_cache = new KImageCache(QStringLiteral("ksudoku-cache"), 3*1024);
	m_mathdokuStyle = false;
	fillNameHashes();
	
	m_themeProvider->discoverThemes(
	    QStringLiteral("themes"), // theme file location
	    QStringLiteral("default") // default theme file name
	);
	const QByteArray themeIdentifier = Settings::theme().toUtf8();
	KGameThemeProvider *provider = themeProvider();
	const QList<const KGameTheme *> themes = provider->themes();
	for (auto* theme : themes) {
		if (theme->identifier() == themeIdentifier) {
		    provider->setCurrentTheme(theme);
		    break;
		}
	}
	loadTheme(provider->currentTheme());
	QObject::connect(m_themeProvider, &KGameThemeProvider::currentThemeChanged, [this](const KGameTheme* theme) {
		loadTheme(theme);
	});

}

Renderer::~Renderer() {
	delete m_themeProvider;
	delete m_cache;
	delete m_renderer;
}

KGameThemeProvider * Renderer::themeProvider() const
{
	return m_themeProvider;
}

bool Renderer::loadTheme(const KGameTheme* theme) {
	bool res = m_renderer->load(theme->graphicsPath());
	qCDebug(KSudokuLog) << "loading" << theme->graphicsPath();
	if(!res)
		return false;

	Settings::setTheme(QString::fromUtf8(theme->identifier()));
	Settings::self()->save();
	
	qCDebug(KSudokuLog) << "discarding cache";
	m_cache->clear();
	
	return true;
}

void Renderer::fillNameHashes() {
    m_borderNames = {
        QString(),
        QStringLiteral("1"),
        QStringLiteral("2"),
        QStringLiteral("12"),
        QStringLiteral("3"),
        QStringLiteral("13"),
        QStringLiteral("23"),
        QStringLiteral("123"),
        QStringLiteral("4"),
        QStringLiteral("14"),
        QStringLiteral("24"),
        QStringLiteral("124"),
        QStringLiteral("34"),
        QStringLiteral("134"),
        QStringLiteral("234"),
        QStringLiteral("1234"),
    };
    m_borderTypes = {
        QString(),
        QStringLiteral("row"),
        QStringLiteral("column"),
        QStringLiteral("block"),
        QStringLiteral("special"),
        QStringLiteral("block"),	// Use block-type borders for cages.
        QStringLiteral("special"),
        QStringLiteral("special"),
        QString(),
        QStringLiteral("row_h"),
        QStringLiteral("column_h"),
        QStringLiteral("block_h"),
        QStringLiteral("special_h"),
        QStringLiteral("block_h"),	// Use block-type borders for cages.
        QStringLiteral("special_h"),
        QStringLiteral("special_h"),
    };
	// m_specialNames are indexed by SpecialType
    m_specialNames = {
        QStringLiteral("cell"),  // SpecialCell
        QStringLiteral("cell_preset"),  // SpecialCellPreset
        QStringLiteral("cell"),  // SpecialCellMarkers
        QStringLiteral("cell_mistake"),  // SpecialCellMistake
        QStringLiteral("cursor"),  // SpecialCursor
        QStringLiteral("valuelist_item"),  // SpecialListItem
        QStringLiteral("valuelist_selector"),  // SpecialListCursor
        QStringLiteral("cursor"),  // SpecialCellHighlight  - using cursor to visually highlight the cell
    };	
    m_special3dNames = {
        QStringLiteral("cell3d"),
        QStringLiteral("cell3d_preset"),
        QStringLiteral("cell3d"),
        QStringLiteral("cell3d_mistake"),
        QStringLiteral("cursor"),
        QStringLiteral("valuelist_item"),
        QStringLiteral("valuelist_selector"),
    };
	// TODO get this hardcoded values from the SVG file
// 	m_markerName << "markers9" << "markers9" //...
}

QPixmap Renderer::renderBackground(const QSize& size) const {
	if(!m_renderer->isValid() || size.isEmpty()) return QPixmap();

	QPixmap pix;
    QString cacheName = QStringLiteral("background_%1x%2").arg(size.width()).arg(size.height());
	if(!m_cache->findPixmap(cacheName, &pix))
	{
		pix = QPixmap(size);
		pix.fill(Qt::transparent);
        QPainter p(&pix);
        m_renderer->render(&p, QStringLiteral("background"));
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
	
    QString cacheName = QStringLiteral("special_%1_%2").arg(m_specialNames[type]).arg(size);
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
        set = QStringLiteral("symbol");
	} else {
        set = QStringLiteral("symbol25");
	}

    QString cacheName = QStringLiteral("%1_%2_%3_%4").arg(set).arg(symbol).arg(size).arg(type);
	QPixmap pix;
	if(!m_cache->findPixmap(cacheName, &pix)) {
		pix = QPixmap(size, size);
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		
        // NOTE fix for Qt's QSvgRenderer size reporting bug
        QRectF r(m_renderer->boundsOnElement(QStringLiteral("symbol_1")));
        QRectF from(m_renderer->boundsOnElement(QStringLiteral("cell_symbol")));
		from.adjust(+0.5,+0.5,-0.5,-0.5); // << this is the fix
		QRectF to(QRectF(0,0,size,size));
		
		r.setTopLeft(fromRectToRect(r.topLeft(), from, to));
		r.setBottomRight(fromRectToRect(r.bottomRight(), from, to));
		
		switch(type) {
			case SymbolPreset:
                if(m_renderer->elementExists(QStringLiteral("%1_%2_preset").arg(set).arg(symbol))) {
                    m_renderer->render(&p, QStringLiteral("%1_%2_preset").arg(set).arg(symbol), r);
				} else {
                    m_renderer->render(&p, QStringLiteral("%1_%2").arg(set).arg(symbol), r);
				}
				break;
			case SymbolEdited:
                if(m_renderer->elementExists(QStringLiteral("%1_%2_edited").arg(set).arg(symbol))) {
                    m_renderer->render(&p, QStringLiteral("%1_%2_edited").arg(set).arg(symbol), r);
				} else {
                    m_renderer->render(&p, QStringLiteral("%1_%2").arg(set).arg(symbol), r);
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
	const qreal dpr = pixmap.devicePixelRatio();
	int scaledSize = m_mathdokuStyle ? (pixmap.width()+1.0*dpr)*0.75 : pixmap.width();
	int offset = m_mathdokuStyle ? (pixmap.width()/dpr+7.0)/8.0 : 0;
	QPixmap symbolPixmap = renderSymbol(symbol, scaledSize, max, type);
	symbolPixmap.setDevicePixelRatio(dpr);
	if(color) {
		// TODO this does not work, need some other way, maybe hardcode color into NumberType
		QPainter p(&symbolPixmap);
		p.setCompositionMode(QPainter::CompositionMode_Multiply);
		p.setBrush(QBrush(QColor(128,128,128,255)));
		p.drawRect(0, 0, scaledSize/dpr, scaledSize/dpr);
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
        set = QStringLiteral("symbol");
	} else {
        set = QStringLiteral("symbol25");
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

    QString groupName = QStringLiteral("markers%1").arg(range);
    QString cacheName = QStringLiteral("%1_%2_%3").arg(groupName).arg(symbol).arg(size);
	QPixmap pix;
	if(!m_cache->findPixmap(cacheName, &pix)) {
		pix = QPixmap(size, size);
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		
		// NOTE fix for Qt's QSvgRenderer size reporting bug
        QRectF r(m_renderer->boundsOnElement(QStringLiteral("%1_%2").arg(groupName).arg(symbol)));
        QRectF from(m_renderer->boundsOnElement(QStringLiteral("cell_%1").arg(groupName)));
		from.adjust(+0.5,+0.5,-0.5,-0.5); // << this is the fix
		QRectF to(QRectF(0,0,size,size));

		r.setTopLeft(fromRectToRect(r.topLeft(), from, to));
		r.setBottomRight(fromRectToRect(r.bottomRight(), from, to));

        m_renderer->render(&p, QStringLiteral("%1_%2").arg(set).arg(symbol), r);
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
	const qreal dpr = pixmap.devicePixelRatio();
	int scaledSize = m_mathdokuStyle ? (pixmap.width()+1.0*dpr)*0.75 : pixmap.width();
	QPixmap symbolPixmap = renderMarker(symbol, range, scaledSize);
	symbolPixmap.setDevicePixelRatio(dpr);
	if(color) {
		QPainter p(&symbolPixmap);
		p.setCompositionMode(QPainter::CompositionMode_Multiply);
		p.setBrush(QBrush(QColor(128,128,128,255)));
		p.drawRect(0, 0, scaledSize/dpr, scaledSize/dpr);
		p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
		p.drawPixmap(0, 0, pixmap);
		p.end();
		return symbolPixmap;
	} else {
		QPainter p(&pixmap);
		// Offset the marker from 0,0 in Mathdoku and Killer Sudoku.
		int offset = m_mathdokuStyle ? (scaledSize/dpr + 7.0)/8.0 : 0;
		p.drawPixmap(offset, 2*offset, symbolPixmap);
		p.end();
		return pixmap;
	}
}

QPixmap Renderer::renderCageLabelOn(QPixmap pixmap, const QString & cageLabel)
{
	// TODO - Do font setup once during resize? Put 0+-x/ in themes?
	const qreal dpr = pixmap.devicePixelRatio();
	const int size = pixmap.width()/dpr;

	QPainter p(&pixmap);
    p.setPen(QStringLiteral("white"));	// Text is white on a dark rectangle.
	p.setBrush(Qt::SolidPattern);

	// Cage label uses top 1/4 of pixmap and text is 1/6 height of pixmap.
	QFont f = p.font();
	f.setBold(true);
	f.setPixelSize((size+5)/6);
	p.setFont(f);

	QFontMetrics fm(f);
        int w = fm.boundingRect(cageLabel).width();	// Width of text.
	int h = fm.height();		// Total height of font.
	int a = fm.ascent();		// Height from baseline of font.
        int m = fm.boundingRect(QLatin1Char('1')).width()/2;	// Left-right margin = 1/2 width of '1'.

	// Paint background rect: text must be visible in light and dark themes.
	p.fillRect(size/6 - m, (size + 3)/4 - a, w + 2*m, h, Qt::darkGray);
	// Note: Origin of text is on baseline to left of first character.
	p.drawText(size/6, (size+3)/4, cageLabel);

	p.end();
	return pixmap;
}

QPixmap Renderer::renderBorder(int border, GroupTypes type, int size) const {
	if(!m_renderer->isValid() || size == 0) return QPixmap();
	
    QString cacheName = QStringLiteral("contour_%1_%2_%3").arg(m_borderTypes[type]).arg(m_borderNames[border]).arg(size);
	QPixmap pix;
	if(!m_cache->findPixmap(cacheName, &pix)) {
		pix = QPixmap(size, size);
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		
		// NOTE fix for Qt's QSvgRenderer size reporting bug
        QRectF r(m_renderer->boundsOnElement(QStringLiteral("%1_%2").arg(m_borderTypes[type]).arg(m_borderNames[border])));
		QRectF from(r.adjusted(+0.5,+0.5,-0.5,-0.5));
		QRectF to(QRectF(0,0,size,size));
		r.setTopLeft(fromRectToRect(r.topLeft(), from, to));
		r.setBottomRight(fromRectToRect(r.bottomRight(), from, to));
		
        m_renderer->render(&p, QStringLiteral("%1_%2").arg(m_borderTypes[type]).arg(m_borderNames[border]), r);
		p.end();
		m_cache->insertPixmap(cacheName, pix);
	}

	return pix;
}

QPixmap Renderer::renderSpecial3D(SpecialType type, int size) const {
	if(!m_renderer->isValid() || size == 0) return QPixmap();

    QString cacheName = QStringLiteral("special_%1_%2").arg(m_special3dNames[type]).arg(size);
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
