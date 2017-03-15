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

#include "gameactions.h"

#include <KLocalizedString>
#include <QAction>
#include <KActionCollection>
#include <QSignalMapper>
#include <QKeySequence>
namespace ksudoku {

GameActions::GameActions(KActionCollection* collection) {
	m_collection = collection;

}

void GameActions::init() {
	m_selectValueMapper = new QSignalMapper(this);
	connect(m_selectValueMapper, SIGNAL(mapped(int)), SIGNAL(selectValue(int)));
	m_enterValueMapper = new QSignalMapper(this);
	connect(m_enterValueMapper, SIGNAL(mapped(int)), SIGNAL(enterValue(int)));
	m_markValueMapper = new QSignalMapper(this);
	connect(m_markValueMapper, SIGNAL(mapped(int)), SIGNAL(markValue(int)));


	// NOTE: Qt::Key_Asterisk cannot be used for Multiply, because it
	//       clashes with Shift+8, used for cancelling '8' or 'h' marker.
	//       Instead, we use 'x' for Multiply, which is OK because Mathdoku
	//       and Killer Sudoku puzzles require only digits 1 to 9.

	// Extras: zero, divide, subtract, add, add cell, end cage, remove cell.
	const Qt::Key extras[] = {Qt::Key_0, Qt::Key_Slash, Qt::Key_Minus,
                            /* Qt::Key_Asterisk, */ Qt::Key_Plus, Qt::Key_Space,
			    Qt::Key_Return};
	QAction * a;
	QList<QKeySequence> shortcuts;
	for(int i = 0; i < 31; ++i) {
		shortcuts.clear();
		a = new QAction(this);
		m_collection->addAction(QString("val-select%1").arg(i+1,2,10,QChar('0')), a);
		a->setText(i18n("Select %1 (%2)", QChar('a'+i), i+1));
		m_selectValueMapper->setMapping(a, i+1);
		connect(a, SIGNAL(triggered(bool)), m_selectValueMapper, SLOT(map()));
		m_actions << a;

		a = new QAction(this);
		m_collection->addAction(QString("val-enter%1").arg(i+1,2,10,QChar('0')), a);
		a->setText(i18n("Enter %1 (%2)", QChar('a'+i), i+1));
		if (i < 25) {
			// Keys A to Y, for Sudoku puzzles.
			shortcuts << QKeySequence(Qt::Key_A + i);
		}
		else {
			// Extras for keying in Mathdoku and Killer Sudoku puzzles.
			shortcuts << QKeySequence(extras[i - 25]);
		}
		if (i < 9) {
			// Keys 1 to 9, for puzzles of order 9 or less.
			shortcuts << QKeySequence(Qt::Key_1 + i);
		}
		m_collection->setDefaultShortcuts(a, shortcuts);
		m_enterValueMapper->setMapping(a, i+1);
		connect(a, SIGNAL(triggered(bool)), m_enterValueMapper, SLOT(map()));
		m_actions << a;
		if (i >= 25) {
			continue;
		}

		shortcuts.clear();
		a = new QAction(this);
		m_collection->addAction(QString("val-mark%1").arg(i+1,2,10,QChar('0')), a);
		a->setText(i18n("Mark %1 (%2)", QChar('a'+i), i+1));
		shortcuts << QKeySequence(Qt::ShiftModifier | (Qt::Key_A + i));
		if(i < 9) {
			shortcuts << QKeySequence(Qt::ShiftModifier | (Qt::Key_1 + i));
		}
		m_collection->setDefaultShortcuts(a, shortcuts);
		m_markValueMapper->setMapping(a, i+1);
		connect(a, SIGNAL(triggered(bool)), m_markValueMapper, SLOT(map()));
		m_actions << a;
	}
	
	a = new QAction(this);
	m_collection->addAction("move_up", a);
	a->setText(i18n("Move Up"));
	m_collection->setDefaultShortcut(a, Qt::Key_Up);
	connect(a, SIGNAL(triggered(bool)), SLOT(moveUp()));
	m_actions << a;

	a = new QAction(this);
	m_collection->addAction("move_down", a);
	a->setText(i18n("Move Down"));
	m_collection->setDefaultShortcut(a, Qt::Key_Down);
	connect(a, SIGNAL(triggered(bool)), SLOT(moveDown()));
	m_actions << a;

	a = new QAction(this);
	m_collection->addAction("move_left", a);
	a->setText(i18n("Move Left"));
	m_collection->setDefaultShortcut(a, Qt::Key_Left);
	connect(a, SIGNAL(triggered(bool)), SLOT(moveLeft()));
	m_actions << a;

	a = new QAction(this);
	m_collection->addAction("move_right", a);
	a->setText(i18n("Move Right"));
	m_collection->setDefaultShortcut(a, Qt::Key_Right);
	connect(a, SIGNAL(triggered(bool)), SLOT(moveRight()));
	m_actions << a;

	a = new QAction(this);
	m_collection->addAction("move_clear_cell", a);
	a->setText(i18n("Clear Cell"));
	m_collection->setDefaultShortcuts(a, QList<QKeySequence>()
		<< QKeySequence(Qt::Key_Backspace)
		<< QKeySequence(Qt::Key_Delete));
	connect(a, SIGNAL(triggered(bool)), SLOT(clearValue()));
	m_actions << a;
}

void GameActions::associateWidget(QWidget* widget) {
	QVector<QAction *>::iterator it;
	for(it = m_actions.begin(); it != m_actions.end(); ++it) {
		widget->addAction(*it);
	}
}

void GameActions::clearValue() {
	emit enterValue(32);	// Delete: not always the same as Qt::Key_0.
}

void GameActions::moveUp() {
	emit move(0, -1);
}

void GameActions::moveDown() {
	emit move(0, +1);
}

void GameActions::moveLeft() {
	emit move(-1, 0);
}

void GameActions::moveRight() {
	emit move(+1, 0);
}

}


