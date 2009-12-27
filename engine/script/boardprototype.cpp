#include "./boardprototype.h"
#include "item.h"

#include <QScriptEngine>

#include <QVariant>

#include "boardwrapper.h"

Q_DECLARE_METATYPE(QVector<QScriptValue>)

BoardPrototype::BoardPrototype(QObject *parent)
	: QObject(parent)
{
}

BoardWrapper* BoardPrototype::thisBoard() const {
	return thisObject().data().toVariant().value<BoardWrapper*>();
}

#include <QDebug>
void BoardPrototype::setItem(const QScriptValue& item, int x, int y, int z, int w) {
	BoardWrapper *wrapper = thisBoard();
	wrapper->setItemAt(item, x, y, z, w);
}

QScriptValue BoardPrototype::items(int x, int y, int z, int w) {
	BoardWrapper *wrapper = thisBoard();
	ItemBoard *board = wrapper->board();
	Q_ASSERT(wrapper);

	// reimplement the ItemBoard::items functionality
	// NOTE must reflect changes in the ItemBoard class
	QVector<QScriptValue> items;
	int min0 = x, min1 = y, min2 = z, min3 = w;
	int max0 = x, max1 = y, max2 = z, max3 = w;
	if(min0 < 0) { min0 = 0; max0 = board->size(0)-1; }
	if(min1 < 0) { min1 = 0; max1 = board->size(1)-1; }
	if(min2 < 0) { min2 = 0; max2 = board->size(2)-1; }
	if(min3 < 0) { min3 = 0; max3 = board->size(3)-1; }

	for(int i3 = min3; i3 <= max3; ++i3) {
		for(int i2 = min2; i2 <= max2; ++i2) {
			for(int i1 = min1; i1 <= max1; ++i1) {
				for(int i0 = min0; i0 <= max0; ++i0) {
					items.append(wrapper->itemAt(i0, i1, i2, i3));
				}
			}
		}
	}

	return engine()->toScriptValue(items);
}

QScriptValue BoardPrototype::split(int x, int y, int z, int w) {
	BoardWrapper *wrapper = thisBoard();
	ItemBoard *board = wrapper->board();
	Q_ASSERT(board);

	int countX = 1, countY = 1, countZ = 1, countW = 1;
	int sizeX = board->size(0), sizeY = board->size(1), sizeZ = board->size(2), sizeW = board->size(3);

	if(x > 0) {
		if(sizeX % x) return context()->throwError(QString("blocks of size (%1,%1,%1,%1) don't fit into board "
			"(x mismatch)").arg(x).arg(y).arg(z).arg(w));
		countX = sizeX / x;
		sizeX = x;
	}
	if(y > 0) {
		if(sizeY % y) return context()->throwError(QString("blocks of size (%1,%1,%1,%1) don't fit into board "
			"(y mismatch)").arg(x).arg(y).arg(z).arg(w));
		countY = sizeY / y;
		sizeY = y;
	}
	if(z > 0) {
		if(sizeZ % z) return context()->throwError(QString("blocks of size (%1,%1,%1,%1) don't fit into board "
			"(z mismatch)").arg(x).arg(y).arg(z).arg(w));
		countZ = sizeZ / z;
		sizeZ = z;
	}
	if(w > 0) {
		if(sizeW % w) return context()->throwError(QString("blocks of size (%1,%1,%1,%1) don't fit into board "
			"(w mismatch)").arg(x).arg(y).arg(z).arg(w));
		countW = sizeW / w;
		sizeW = w;
	}

	QVector<QScriptValue> blocks;
	for(int ix = 0; ix < countX; ++ix) {
	for(int iy = 0; iy < countY; ++iy) {
	for(int iz = 0; iz < countZ; ++iz) {
	for(int iw = 0; iw < countW; ++iw) {
		QVector<QScriptValue> block;
		block.reserve(sizeX*sizeY*sizeZ*sizeW);
		for(int jx = 0; jx < sizeX; ++jx) {
		for(int jy = 0; jy < sizeY; ++jy) {
		for(int jz = 0; jz < sizeZ; ++jz) {
		for(int jw = 0; jw < sizeW; ++jw) {
			QScriptValue item = wrapper->itemAt(ix*sizeX+jx, iy*sizeY+jy, iz*sizeZ+jz, iw*sizeW+jw);
			block << item;
		}}}}
		blocks << engine()->toScriptValue(block);
	}}}}

	return engine()->toScriptValue(blocks);
}

#include "boardprototype.moc"

