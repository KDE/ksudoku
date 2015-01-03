/***************************************************************************
 *   Copyright 2008      Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#ifndef _KSUDOKU_GAMEACTIONS_H_
#define _KSUDOKU_GAMEACTIONS_H_

#include <QVector>
#include <QObject>

class QAction;
class KActionCollection;
class QSignalMapper;

namespace ksudoku {

class GameActions : public QObject {
	Q_OBJECT
public:
	GameActions(KActionCollection* collection);
	void init();
	void associateWidget(QWidget* widget);
signals:
	void selectValue(int value);
	void enterValue(int value = -1);
	void markValue(int value = -1);
	void move(int dx, int dy);
private slots:
	void clearValue();
	void moveUp();
	void moveDown();
	void moveLeft();
	void moveRight();
private:
	KActionCollection* m_collection;
	QSignalMapper* m_selectValueMapper;
	QSignalMapper* m_enterValueMapper;
	QSignalMapper* m_markValueMapper;
	QVector<QAction *> m_actions;
};

}

#endif
