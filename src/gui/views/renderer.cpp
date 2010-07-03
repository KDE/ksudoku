#include "renderer.h"

#include <QSvgRenderer>
#include <KStandardDirs>
#include <kpixmapcache.h>
#include <KGameTheme>

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
	m_cache = new KPixmapCache("ksudoku-cache");
	m_cache->setCacheLimit(3*1024);
	
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
		m_cache->discard();
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
	m_borderTypes << "special";
	m_borderTypes << "special";
	m_borderTypes << "special";
	m_borderTypes << QString();
	m_borderTypes << "row_h";
	m_borderTypes << "column_h";
	m_borderTypes << "block_h";
	m_borderTypes << "special_h";
	m_borderTypes << "special_h";
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
	if(!m_cache->find(cacheName, pix))
	{
		pix = QPixmap(size);
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		m_renderer->render(&p, "background");
		p.end();
		m_cache->insert(cacheName, pix);
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
	if(!m_cache->find(cacheName, pix)) {
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
		m_cache->insert(cacheName, pix);
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
	if(!m_cache->find(cacheName, pix)) {
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
		m_cache->insert(cacheName, pix);
	}

	return pix;
}

QPixmap Renderer::renderSymbolOn(QPixmap pixmap, int symbol, int color, int max, SymbolType type) const {
	int size = pixmap.width();
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
		p.drawPixmap(0, 0, symbolPixmap);
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
	if(!m_cache->find(cacheName, pix)) {
		qDebug() << cacheName;
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
		m_cache->insert(cacheName, pix);
	}

	return pix;
}

QPixmap Renderer::renderMarkerOn(QPixmap pixmap, int symbol, int range, int color) const {
	// TODO maybe it would be good to directly integrate the renderMarker implementation and
	// make renderMarker be based on this method. (same for renderSymbol and renderSymbolOn)
	int size = pixmap.width();
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
		p.drawPixmap(0, 0, symbolPixmap);
		p.end();
		return pixmap;
	}
}

QPixmap Renderer::renderBorder(int border, GroupTypes type, int size) const {
	if(!m_renderer->isValid() || size == 0) return QPixmap();
	
	//show highlights only if they are set
	if(!Settings::showHighlights()) type &= ~GroupHighlight;
	
	QString cacheName = QString("contour_%1_%2_%3").arg(m_borderTypes[type]).arg(m_borderNames[border]).arg(size);
	QPixmap pix;
	if(!m_cache->find(cacheName, pix)) {
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
		m_cache->insert(cacheName, pix);
	}

	return pix;
}

QPixmap Renderer::renderSpecial3D(SpecialType type, int size) const {
	if(!m_renderer->isValid() || size == 0) return QPixmap();

	QString cacheName = QString("special_%1_%2").arg(m_special3dNames[type]).arg(size);
	QPixmap pix;
	if(!m_cache->find(cacheName, pix)) {
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
		m_cache->insert(cacheName, pix);
	}

	return pix;
}

}
