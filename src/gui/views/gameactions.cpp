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

#include "gameactions.h"

#include <KLocale>
#include <KAction>
#include <KActionCollection>
#include <QSignalMapper>

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


	KAction* a;
	KShortcut shortcut;
	for(int i = 0; i < 25; ++i) {
		a = new KAction(this);
		m_collection->addAction(QString("val-select%1").arg(i+1,2,10,QChar('0')), a);
		a->setText(i18n("Select %1 (%2)", QChar('a'+i), i+1));
		m_selectValueMapper->setMapping(a, i+1);
		connect(a, SIGNAL(triggered(bool)), m_selectValueMapper, SLOT(map()));
		m_actions << a;

		a = new KAction(this);
		m_collection->addAction(QString("val-enter%1").arg(i+1,2,10,QChar('0')), a);
		a->setText(i18n("Enter %1 (%2)", QChar('a'+i), i+1));
		shortcut = a->shortcut();
		shortcut.setPrimary( Qt::Key_A + i);
		if(i < 9) {
			shortcut.setAlternate( Qt::Key_1 + i);
		}
		a->setShortcut(shortcut);
		m_enterValueMapper->setMapping(a, i+1);
		connect(a, SIGNAL(triggered(bool)), m_enterValueMapper, SLOT(map()));
		m_actions << a;

		a = new KAction(this);
		m_collection->addAction(QString("val-mark%1").arg(i+1,2,10,QChar('0')), a);
		a->setText(i18n("Mark %1 (%2)", QChar('a'+i), i+1));
		shortcut = a->shortcut();
		shortcut.setPrimary( Qt::ShiftModifier | Qt::Key_A + i);
		if(i < 9) {
			shortcut.setAlternate( Qt::ShiftModifier | Qt::Key_1 + i);
		}
		a->setShortcut(shortcut);
		m_markValueMapper->setMapping(a, i+1);
		connect(a, SIGNAL(triggered(bool)), m_markValueMapper, SLOT(map()));
		m_actions << a;
	}
	
	a = new KAction(this);
	m_collection->addAction("move_up", a);
	a->setText(i18n("Move Up"));
	a->setShortcut(Qt::Key_Up);
	connect(a, SIGNAL(triggered(bool)), SLOT(moveUp()));
	m_actions << a;

	a = new KAction(this);
	m_collection->addAction("move_down", a);
	a->setText(i18n("Move Down"));
	a->setShortcut(Qt::Key_Down);
	connect(a, SIGNAL(triggered(bool)), SLOT(moveDown()));
	m_actions << a;

	a = new KAction(this);
	m_collection->addAction("move_left", a);
	a->setText(i18n("Move Left"));
	a->setShortcut(Qt::Key_Left);
	connect(a, SIGNAL(triggered(bool)), SLOT(moveLeft()));
	m_actions << a;

	a = new KAction(this);
	m_collection->addAction("move_right", a);
	a->setText(i18n("Move Right"));
	a->setShortcut(Qt::Key_Right);
	connect(a, SIGNAL(triggered(bool)), SLOT(moveRight()));
	m_actions << a;

	a = new KAction(this);
	m_collection->addAction("move_clear_cell", a);
	a->setText(i18n("Clear Cell"));
	shortcut = a->shortcut();
	shortcut.setPrimary(Qt::Key_Backspace);
	shortcut.setAlternate(Qt::Key_Delete);
	a->setShortcut(shortcut);
	connect(a, SIGNAL(triggered(bool)), SLOT(clearValue()));
	m_actions << a;
}

void GameActions::associateWidget(QWidget* widget) {
	QVector<KAction*>::iterator it;
	for(it = m_actions.begin(); it != m_actions.end(); ++it) {
		widget->addAction(*it);
	}
}

void GameActions::clearValue() {
	emit enterValue(0);
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

#include "gameactions.moc"
