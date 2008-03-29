#ifndef _KSUDOKU_RENDERER_H_
#define _KSUDOKU_RENDERER_H_

#include <QVector>
// #include <QPixmap>
#include <QString>

class QPixmap;
class QSize;
class KSvgRenderer;
class KPixmapCache;

namespace ksudoku {

enum GroupType {
	GroupNone              = 0x00,
	GroupRow               = 0x01,
	GroupColumn            = 0x02,
	GroupBlock             = 0x03,
	GroupSpecial           = 0x04,
	GroupUnhighlightedMask = 0x07,
	GroupHighlight         = 0x08
};

enum SpecialType {
	SpecialCell        = 0x00,
	SpecialCellPreset  = 0x01,
	SpecialCellMarkers = 0x02,
	SpecialCellMistake = 0x03,
	SpecialCursor      = 0x04
};

Q_DECLARE_FLAGS(GroupTypes, GroupType)

class Renderer {

enum SupportFlag {
	HasRow               = 0x0001,
	HasColumn            = 0x0002,
	HasBlock             = 0x0004,
	HasSpecial           = 0x0008,
	HasRowHighlight      = 0x0010,
	HasColumnHighlight   = 0x0020,
	HasBlockHighlight    = 0x0040,
	HasSpecialHighlight  = 0x0080,
	HasBlockCenter       = 0x0100,
	HasSpecialCenter     = 0x0200,
	ContoursInBackground = 0x1000
};

public:
	static Renderer* instance();

	QPixmap renderBackground(const QSize& size) const;
	QPixmap renderSpecial(SpecialType type, int size) const;

	QPixmap renderBorder(int border, GroupTypes type, int size) const;

	QPixmap renderSymbol(int symbol, int size) const;
	QPixmap renderSymbolOn(QPixmap pixmap, int symbol, int color) const;
private:
	Renderer();
	~Renderer();
private:
	void fillNameHashes();
private:
	bool m_hasRowAndColumn : 1;
	bool m_hasRowAndColumnHighlight : 1;
	bool m_hasBlock : 1;
	bool m_hasBlockHighlight : 1;
	bool m_hasSpecial : 1;
	bool m_hasSpecialHighlight : 1;
	QVector<QString> m_borderNames;
	QVector<QString> m_borderTypes;
	QVector<QString> m_specialNames;
	KSvgRenderer* m_renderer;
	KPixmapCache* m_cache;
};

}

#endif
