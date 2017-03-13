/***************************************************************************
 *   Copyright 2007      Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#include "ksudoku_logging.h"
#include "gamevariants.h"
#include "ksudokugame.h"
#include "serializer.h"

#include <QDebug>
#include <KMessageBox>
#include <KLocalizedString>
#include <QPainter>
#include <QIcon>
#include <QEvent>



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

GameVariantDelegate::GameVariantDelegate(QObject* parent, QWidget * viewport)
	: QItemDelegate(parent),
	  m_viewport(viewport)
{
}

QSize GameVariantDelegate::sizeHint(const QStyleOptionViewItem& option,
				    const QModelIndex& index) const
{
	Q_UNUSED(index);

	// Fit a varying number of columns into the list-view.
	int viewportWidth = m_viewport->width();
	int hintWidth     = m_minWidth;
	int nItemsPerRow  = viewportWidth / m_minWidth;

	// Expand the hinted width, so that the columns will fill the viewport.
	if (nItemsPerRow > 0) {
	    int adjustment = (viewportWidth % nItemsPerRow) ? 0 : 1;
	    // The adjustment works around a graphics glitch, when nItemsPerRow
	    // exactly divides viewportWidth and instead of nItemsPerRow columns
	    // we suddenly get one column less, plus a large empty space, even
	    // though QListView::spacing() is zero.
	    hintWidth      = (viewportWidth - adjustment) / nItemsPerRow;
	}

	// Set the height to contain the icon or 4 lines of text.
	QSize size (hintWidth, 0);
	int fontHeight = option.fontMetrics.height();
	if (m_iconHeight < fontHeight*4) {
		size.setHeight (fontHeight*4 + m_separatorPixels*3);
	}
	else {
		size.setHeight (m_iconHeight + m_separatorPixels*2);
	}
	return size;
}

void GameVariantDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {

	QColor bkgColor;

	// Calculate item's column num in list-view. Add 1 in odd-numbered rows.
	int nItemsPerRow = qMax (m_viewport->width() / option.rect.width(), 1);
	int oddColumn = index.row() % nItemsPerRow;
	oddColumn = oddColumn + ((index.row() / nItemsPerRow) % 2);

	if (option.state & QStyle::State_Selected) {
		bkgColor = option.palette.color(QPalette::Highlight);
	} else if (oddColumn % 2) {
		// The shading alternates along each row in the list view and
		// each odd-numbered row starts with a shaded item, so we have
		// a checkerboard pattern, regardless of whether nItemsPerRow
		// is odd or even (see the above calculation).
		bkgColor = option.palette.color(QPalette::AlternateBase);
	} else {
		bkgColor = option.palette.color(QPalette::Base);
	}
	painter->fillRect(option.rect, bkgColor);

	QRect contentRect = option.rect.adjusted(m_leftMargin, m_separatorPixels, -m_rightMargin, -m_separatorPixels);

	// Show icon

	QPixmap iconPixmap = QIcon::fromTheme(icon(index)).pixmap(m_iconWidth, m_iconHeight);
	painter->drawPixmap(contentRect.left(), (contentRect.height() - iconPixmap.height()) / 2 + contentRect.top(), iconPixmap);
	contentRect.adjust(iconPixmap.width() + m_separatorPixels*2, 0, 0, 0);

// 	// Show configuration icon
// 	if(configurable(index)) {
// 		QPixmap configPixmap = QIcon::fromTheme( QLatin1String( "configure" ) ).pixmap(32, 32);
// 		painter->drawPixmap(contentRect.right() - configPixmap.width(), (contentRect.height() - configPixmap.height()) / 2 + contentRect.top(), configPixmap);
// 		contentRect.adjust(0, 0, -(configPixmap.width() + separatorPixels), 0);
// 	}

	// Show Title
	QFont titleFont(painter->font());
	titleFont.setPointSize(titleFont.pointSize() + 2);
	titleFont.setWeight(QFont::Bold);

	QPen previousPen(painter->pen());
	// TODO: don't we have a function for 'color1 similar color2'
	if(previousPen.color() == bkgColor) {
		QPen p(previousPen);
		int color = bkgColor.rgb();
		color = ~color | 0xff000000;
		p.setColor(color);
		painter->setPen(p);
	}

	QFont previousFont(painter->font());
	painter->setFont(titleFont);
	QString titleStr = painter->fontMetrics().elidedText(title(index), Qt::ElideRight, contentRect.width());
	painter->drawText(contentRect, titleStr);
	contentRect.adjust(0, m_separatorPixels + painter->fontMetrics().height(), 0, 0);
	painter->setFont(previousFont);

	// Show Description
	QTextOption wrap(Qt::AlignLeft);
	wrap.setWrapMode(QTextOption::WordWrap);
	QString descrStr = description(index);
	painter->drawText(contentRect, descrStr, wrap);

	painter->setPen(previousPen);
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
	: GameVariant(name, collection), m_order(order), m_graph(0)
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

Game SudokuGame::startEmpty() {
	if(!m_graph) {
		m_graph = new SKGraph(m_order, TypeSudoku);
		m_graph->initSudoku();
	}

	Puzzle* puzzle = new Puzzle(m_graph, false);
	puzzle->init();

	return Game(puzzle);
}

Game SudokuGame::createGame(int difficulty, int symmetry) {
	if(!m_graph) {
		m_graph = new SKGraph(m_order, TypeSudoku);
		m_graph->initSudoku();
	}
	
	Puzzle* puzzle = new Puzzle(m_graph, true);
	puzzle->init(difficulty, symmetry);

	return Game(puzzle);
}

KsView* SudokuGame::createView(const Game& /*game*/) const {
	qCDebug(KSudokuLog) << "KsView* ksudoku::SudokuGame::createView()";
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// class RoxdokuGame
///////////////////////////////////////////////////////////////////////////////

RoxdokuGame::RoxdokuGame(const QString& name, uint order, GameVariantCollection* collection)
	: GameVariant(name, collection), m_order(order), m_graph(0)
{
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

Game RoxdokuGame::startEmpty() {
	if(!m_graph) {
		m_graph = new SKGraph(m_order, TypeRoxdoku);
		m_graph->initRoxdoku();
	}

	Puzzle* puzzle = new Puzzle(m_graph, false);
	puzzle->init();

	return Game(puzzle);
}

Game RoxdokuGame::createGame(int difficulty, int symmetry) {
	if(!m_graph) {
		m_graph = new SKGraph(m_order, TypeRoxdoku);
		m_graph->initRoxdoku();
	}

	Puzzle* puzzle = new Puzzle(m_graph, true);
	puzzle->init(difficulty, symmetry);

	return Game(puzzle);
}

KsView* RoxdokuGame::createView(const Game& /*game*/) const {
	qCDebug(KSudokuLog) << "KsView* ksudoku::RoxdokuGame::createView()";
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// class CustomGame
///////////////////////////////////////////////////////////////////////////////

CustomGame::CustomGame(const QString& name, const QUrl& url,
                       GameVariantCollection* collection)
	: GameVariant(name, collection), m_url(url), m_graph(0)
{
	m_symmetry = 0;
}

bool CustomGame::canConfigure() const {
	return false;
}

bool CustomGame::configure() {
	return false;
}

bool CustomGame::canStartEmpty() const {
	return true;
}

Game CustomGame::startEmpty() {
	if (! createSKGraphObject()) {
	    return Game();
	}
	Puzzle* puzzle = new Puzzle(m_graph, false);
	puzzle->init();

	return Game(puzzle);
}

Game CustomGame::createGame(int difficulty, int symmetry) {
	if (! createSKGraphObject()) {
	    return Game();
	}
	Puzzle* puzzle = new Puzzle(m_graph, true);
	puzzle->init(difficulty, symmetry);

	return Game(puzzle);
}

KsView* CustomGame::createView(const Game& /*game*/) const {
	qCDebug(KSudokuLog) << "KsView* ksudoku::CustomGame::createView()";
	return 0;
}

bool CustomGame::createSKGraphObject()
{
	if ((m_graph != 0) && ((m_graph->specificType() == Mathdoku) ||
		    (m_graph->specificType() == KillerSudoku))) {
	    delete m_graph;	// Re-create SKGraph for every Mathdoku or
	    m_graph = 0;	// Killer Sudoku game (re-inits cages and size).
	}
	if (m_graph == 0) {
	    m_graph = ksudoku::Serializer::loadCustomShape(m_url, 0, 0);
	}
	return (m_graph != 0);	// True if the shapes/*.xml file loaded OK.
}

}
