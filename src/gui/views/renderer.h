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

#ifndef _KSUDOKU_RENDERER_H_
#define _KSUDOKU_RENDERER_H_

#include <KImageCache>

#include <QList>
#include <QString>

class KGameTheme;
class KGameThemeProvider;

class QPixmap;
class QSize;
class QSvgRenderer;

namespace ksudoku {

enum GroupType {
	GroupNone              = 0x00,
	GroupRow               = 0x01,
	GroupColumn            = 0x02,
	GroupBlock             = 0x03,
	GroupSpecial           = 0x04,
	GroupCage              = 0x05, // For Mathdoku and Killer Sudoku only.
	GroupUnhighlightedMask = 0x07,
	GroupHighlight         = 0x08
};

enum SpecialType {
	SpecialCell        = 0x00,
	SpecialCellPreset  = 0x01,
	SpecialCellMarkers = 0x02,
	SpecialCellMistake = 0x03,
	SpecialCursor      = 0x04,
	SpecialListItem    = 0x05,
	SpecialListCursor  = 0x06,
	SpecialCellHighlight = 0x07
};

enum SymbolType {
	SymbolPreset	= 0x00,
	SymbolEdited	= 0x01
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

	bool loadTheme(const KGameTheme* theme);

	// In Mathdoku style, symbols and markers are drawn smaller than usual
	// and re-positioned, to allow space at the top left of a cell for a
	// cage label. The label contains the value and operator clues.
	void setMathdokuStyle(bool onOff) { m_mathdokuStyle = onOff; }

	QPixmap renderBackground(const QSize& size) const;
	QPixmap renderSpecial(SpecialType type, int size) const;

	QPixmap renderBorder(int border, GroupTypes type, int size) const;

	QPixmap renderSymbol(int symbol, int size, int max, SymbolType type) const;
    QPixmap renderSymbolOn(QPixmap pixmap, int symbol, int color, int max, SymbolType type) const;

	QPixmap renderMarker(int symbol, int range, int size) const;
    QPixmap renderMarkerOn(QPixmap pixmap, int symbol, int range, int color) const;

    QPixmap renderCageLabelOn(QPixmap pixmap, const QString & cageLabel);

	QPixmap renderSpecial3D(SpecialType type, int size) const;

	KGameThemeProvider *themeProvider() const;

private:
	Renderer();
	~Renderer();
private:
	void fillNameHashes();
private:
	// TODO - The next 6 data items are unused.
/*
	bool m_hasRowAndColumn : 1;
	bool m_hasRowAndColumnHighlight : 1;
	bool m_hasBlock : 1;
	bool m_hasBlockHighlight : 1;
	bool m_hasSpecial : 1;
	bool m_hasSpecialHighlight : 1;
*/
	QList<QString> m_borderNames;
	QList<QString> m_borderTypes;
	QList<QString> m_specialNames;
	QList<QString> m_special3dNames;
	QList<QString> m_markerNames;
	KGameThemeProvider *m_themeProvider;
	QSvgRenderer* m_renderer;
	KImageCache* m_cache;
	bool m_mathdokuStyle;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(ksudoku::GroupTypes)

#endif
