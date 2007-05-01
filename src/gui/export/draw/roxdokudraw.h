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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KSUDOKUROXDOKUDRAW_H
#define KSUDOKUROXDOKUDRAW_H

#include "drawbase.h"

namespace ksudoku {

/**
 * Draw roxdoku game to QPaintdevice
 */
class RoxdokuDraw : public DrawBase
{
public:
	RoxdokuDraw(Puzzle const& puzzle, Symbols const& symbols);
	~RoxdokuDraw();

	///@TODO implement me
	void drawRaster(QPainter& p, int width, int height) const {;}
	///@TODO implement me
	void drawValues(QPainter& p, int width, int height) const {;}
};

}

#endif
