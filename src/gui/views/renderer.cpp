 #include "renderer.h"

#include <KSvgRenderer>
#include <KStandardDirs>
#include <kpixmapcache.h>

#include <QPixmap>
#include <QPainter>

#include <QtDebug>

namespace ksudoku {

Renderer* Renderer::instance() {
	static Renderer instance;
	return &instance;
}
	
Renderer::Renderer() {
	m_cache = new KPixmapCache("ksudoku-cache");
	m_cache->setCacheLimit(3*1024);
	
	m_renderer = new KSvgRenderer(KStandardDirs::locate("appdata", "themes/ksudoku_sample.svg"));
	fillNameHashes();
}

Renderer::~Renderer() {
	delete m_cache;
	delete m_renderer;
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

QPixmap Renderer::renderSymbol(int symbol, int size) const {
	if(!m_renderer->isValid() || size == 0) return QPixmap();

	QString cacheName = QString("symbol_%1_%2").arg(symbol).arg(size);
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
		
		m_renderer->render(&p, QString("symbol_%1").arg(symbol), r);
		p.end();
		m_cache->insert(cacheName, pix);
	}

    return pix;
}

QPixmap Renderer::renderSymbolOn(QPixmap pixmap, int symbol, int color) const {
	int size = pixmap.width();
	QPixmap symbolPixmap = renderSymbol(symbol, size);
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

QPixmap Renderer::renderMarker(int symbol, int range, int size) const {
	if(!m_renderer->isValid() || size == 0) return QPixmap();

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

		qDebug() << r << from << to;
		
		r.setTopLeft(fromRectToRect(r.topLeft(), from, to));
		r.setBottomRight(fromRectToRect(r.bottomRight(), from, to));

		qDebug() << r;
		
		m_renderer->render(&p, QString("symbol_%1").arg(symbol), r);
		p.end();
		m_cache->insert(cacheName, pix);
	}

    return pix;
}

QPixmap Renderer::renderMarkerOn(QPixmap pixmap, int symbol, int range, int color) const {
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

}
