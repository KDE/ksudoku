/***************************************************************************
 *   Copyright 2007      Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#include "gamevariants.h"
#include "ksudokugame.h"
#include "serializer.h"

#include <QtDebug>
#include <KMessageBox>
#include <KLocale>
#include <QPainter>
#include <KIconLoader>
#include <QEvent>

#include "gamevariants.moc"

#include "puzzle.h"


namespace ksudoku {

///////////////////////////////////////////////////////////////////////////////
// class GameVariant
///////////////////////////////////////////////////////////////////////////////

GameVariant::GameVariant(const QString& name, GameVariantCollection* collection)
	: m_name(name)
{
	if(collection)
		collection->addVariant(this);
}

void GameVariant::setDescription(const QString& descr) {
	m_description = descr;
}

void GameVariant::setIcon(const QString& icon) {
	m_icon = icon;
}

///////////////////////////////////////////////////////////////////////////////
// class GameVariantCollection
///////////////////////////////////////////////////////////////////////////////

GameVariantCollection::GameVariantCollection(QObject* parent, bool autoDel)
	: QAbstractListModel(parent), m_autoDelete(autoDel)
{
}

GameVariantCollection::~GameVariantCollection() {
	if(m_autoDelete)
		qDeleteAll(m_variants);
	m_variants.clear();
}

void GameVariantCollection::addVariant(GameVariant* variant) {
	int count = m_variants.count();
	beginInsertRows(QModelIndex(), count, count);
	m_variants.append(variant);
	endInsertRows();
	emit newVariant(variant);
}

int GameVariantCollection::rowCount(const QModelIndex& parent) const {
	Q_UNUSED(parent);
	return m_variants.count();
}

QModelIndex GameVariantCollection::index(int row, int column, const QModelIndex &parent) const {
	Q_UNUSED(parent);
	if ((row < 0) || (row >= m_variants.count()))
		return QModelIndex();
	return createIndex(row, column, m_variants[row]);
}

QVariant GameVariantCollection::data(const QModelIndex &index, int role) const {
	if (!index.isValid() || index.row() >= m_variants.count())
		return QVariant();

	if (!index.internalPointer())
		return QVariant();

	GameVariant* gameVariant = static_cast<GameVariant*>(index.internalPointer());

	switch(role) {
		case Qt::DisplayRole:
			return gameVariant->name();
		case Qt::DecorationRole:
			return gameVariant->icon();
		case GameVariantDelegate::Description:
			return gameVariant->description();
	}

	return QVariant();
}

GameVariant* GameVariantCollection::variant(const QModelIndex& index) const {
	return static_cast<GameVariant*>(index.internalPointer());
}

///////////////////////////////////////////////////////////////////////////////
// class GameVariantDelegate
////////////77/////////////////////////////////////////////////////////////////

GameVariantDelegate::GameVariantDelegate(QObject* parent)
	: QItemDelegate(parent)
{
}

QSize GameVariantDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
	Q_UNUSED(option);
	Q_UNUSED(index);
	QSize size(m_leftMargin+m_iconWidth+m_rightMargin+m_separatorPixels*2, m_iconHeight+m_separatorPixels*2);

	QRect contentRect = option.rect.adjusted(m_leftMargin, m_separatorPixels, -m_rightMargin, -m_separatorPixels);
	contentRect.adjust(m_iconWidth + m_separatorPixels*2, 0, 0, 0);

	QFont titleFont(option.font);
	titleFont.setPointSize(titleFont.pointSize() + 2);
	titleFont.setWeight(QFont::Bold);

	QString titleStr = option.fontMetrics.elidedText(title(index), Qt::ElideRight, contentRect.width());
	QSize titleSize = option.fontMetrics.size( Qt::TextSingleLine, titleStr );
	contentRect.adjust(0, m_separatorPixels + option.fontMetrics.height(), 0, 0);

	// Show Description
	QString descrStr = option.fontMetrics.elidedText(description(index), Qt::ElideRight, contentRect.width());
	QSize descrSize = option.fontMetrics.size( Qt::TextSingleLine, descrStr );

	if( descrSize.width() > titleSize.width() )
		size.setWidth(size.width() + descrSize.width());
	else
		size.setWidth(size.width() + titleSize.width());

	if( titleSize.height()+descrSize.height()+m_separatorPixels*2 > m_iconWidth )
		size.setHeight( titleSize.height()+option.fontMetrics.height()+descrSize.height()+m_separatorPixels*4 );
	return size;
}

void GameVariantDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {

	// show background
	if (option.state & QStyle::State_Selected) {
		painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));
	} else if(index.row() % 2) {
		painter->fillRect(option.rect, option.palette.color(QPalette::AlternateBase));
	} else {
		painter->fillRect(option.rect, option.palette.color(QPalette::Base));
	}

	QRect contentRect = option.rect.adjusted(m_leftMargin, m_separatorPixels, -m_rightMargin, -m_separatorPixels);

	// Show icon

	QPixmap iconPixmap = KIcon(icon(index), KIconLoader::global()).pixmap(m_iconWidth, m_iconHeight);
	painter->drawPixmap(contentRect.left(), (contentRect.height() - iconPixmap.height()) / 2 + contentRect.top(), iconPixmap);
	contentRect.adjust(iconPixmap.width() + m_separatorPixels*2, 0, 0, 0);

// 	// Show configuration icon
// 	if(configurable(index)) {
// 		QPixmap configPixmap = KIcon("configure", KIconLoader::global()).pixmap(32, 32);
// 		painter->drawPixmap(contentRect.right() - configPixmap.width(), (contentRect.height() - configPixmap.height()) / 2 + contentRect.top(), configPixmap);
// 		contentRect.adjust(0, 0, -(configPixmap.width() + separatorPixels), 0);
// 	}

	// Show Title
	QFont titleFont(painter->font());
	titleFont.setPointSize(titleFont.pointSize() + 2);
	titleFont.setWeight(QFont::Bold);

	QFont previousFont(painter->font());
	painter->setFont(titleFont);
	QString titleStr = painter->fontMetrics().elidedText(title(index), Qt::ElideRight, contentRect.width());
	painter->drawText(contentRect, titleStr);
	contentRect.adjust(0, m_separatorPixels + painter->fontMetrics().height(), 0, 0);
	painter->setFont(previousFont);

	// Show Description
	QString descrStr = painter->fontMetrics().elidedText(description(index), Qt::ElideRight, contentRect.width());
	painter->drawText(contentRect, descrStr);
}

QString GameVariantDelegate::title(const QModelIndex& index) const {
	return index.model()->data(index, Qt::DisplayRole).toString();
}

QString GameVariantDelegate::description(const QModelIndex& index) const {
	return index.model()->data(index, Description).toString();
}

QString GameVariantDelegate::icon(const QModelIndex& index) const {
	return index.model()->data(index, Qt::DecorationRole).toString();
}

bool GameVariantDelegate::configurable(const QModelIndex& index) const {
	const GameVariantCollection* collection = dynamic_cast<const GameVariantCollection*>(index.model());
	if(!collection) return false;

	return collection->variant(index)->canConfigure();
}

bool GameVariantDelegate::eventFilter(QObject* watched, QEvent* event) {
	if(event->type() == QEvent::MouseButtonPress) {
		return true;
	}

	// TODO insert code for handling clicks on buttons in items.

	return QItemDelegate::eventFilter(watched, event);
}

///////////////////////////////////////////////////////////////////////////////
// class SudokuGame
///////////////////////////////////////////////////////////////////////////////

SudokuGame::SudokuGame(const QString& name, uint order, GameVariantCollection* collection)
	: GameVariant(name, collection), m_order(order), m_solver(0)
{
	// TODO load from settings
	m_symmetry = 0;
}

bool SudokuGame::canConfigure() const {
	return true;
}

bool SudokuGame::configure() {
	KMessageBox::information(0, i18n("Configuration not yet implemented"), "");
	return false;
}

bool SudokuGame::canStartEmpty() const {
	return true;
}

Game SudokuGame::startEmpty() const {
	if(!m_solver) {
		m_solver = new SKSolver(m_order, false);
		m_solver->init();
	}

	Puzzle* puzzle = new Puzzle(m_solver, false);
	puzzle->init();

	return Game(puzzle);
}

Game SudokuGame::createGame(int difficulty) const {
	if(!m_solver) {
		m_solver = new SKSolver(m_order, false);
		m_solver->init();
	}

	Puzzle* puzzle = new Puzzle(m_solver, true);
	puzzle->init(difficulty, m_symmetry);

	return Game(puzzle);
}

KsView* SudokuGame::createView(const Game& /*game*/) const {
	qDebug() << "KsView* ksudoku::SudokuGame::createView()";
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// class RoxdokuGame
///////////////////////////////////////////////////////////////////////////////

RoxdokuGame::RoxdokuGame(const QString& name, uint order, GameVariantCollection* collection)
	: GameVariant(name, collection), m_order(order), m_solver(0)
{
	// TODO load from settings
	m_symmetry = 0;
}

bool RoxdokuGame::canConfigure() const {
	return true;
}

bool RoxdokuGame::configure() {
	KMessageBox::information(0, i18n("Configuration not yet implemented"), "");
	return false;
}

bool RoxdokuGame::canStartEmpty() const {
	return true;
}

Game RoxdokuGame::startEmpty() const {
	if(!m_solver) {
		m_solver = new SKSolver(m_order, true);
		m_solver->init();
	}

	Puzzle* puzzle = new Puzzle(m_solver, false);
	puzzle->init();

	return Game(puzzle);
}

Game RoxdokuGame::createGame(int difficulty) const {
	if(!m_solver) {
		m_solver = new SKSolver(m_order, true);
		m_solver->init();
	}

	Puzzle* puzzle = new Puzzle(m_solver, true);
	puzzle->init(difficulty, m_symmetry);

	return Game(puzzle);
}

KsView* RoxdokuGame::createView(const Game& /*game*/) const {
	qDebug() << "KsView* ksudoku::RoxdokuGame::createView()";
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// class CustomGame
///////////////////////////////////////////////////////////////////////////////

CustomGame::CustomGame(const QString& name, const KUrl& url,
                       GameVariantCollection* collection)
	: GameVariant(name, collection), m_url(url), m_solver(0)
{ }

bool CustomGame::canConfigure() const {
	return false;
}

bool CustomGame::configure() {
	return false;
}

bool CustomGame::canStartEmpty() const {
	return true;
}

Game CustomGame::startEmpty() const {
	if(!m_solver) {
		m_solver = ksudoku::Serializer::loadCustomShape(m_url, 0, 0);

		if(!m_solver) return Game();
	}

	Puzzle* puzzle = new Puzzle(m_solver, false);
	puzzle->init();

	return Game(puzzle);
}

Game CustomGame::createGame(int difficulty) const {
	if(!m_solver) {
		m_solver = ksudoku::Serializer::loadCustomShape(m_url, 0, 0);

		if(!m_solver) return Game();
	}

	Puzzle* puzzle = new Puzzle(m_solver, true);
	puzzle->init(difficulty, 1);

	return Game(puzzle);
}

KsView* CustomGame::createView(const Game& /*game*/) const {
	qDebug() << "KsView* ksudoku::CustomGame::createView()";
	return 0;
}

}
