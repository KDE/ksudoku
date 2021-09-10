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

#ifndef _KSUDOKUGAMEVARIANTS_H_
#define _KSUDOKUGAMEVARIANTS_H_

#include <QAbstractListModel>
#include <QItemDelegate>
#include <QList>
#include <QObject>
#include <QUrl>

class SKGraph;
namespace ksudoku {

class KsView;
class Game;
class GameVariantCollection;
class GameVariant {
public:
	explicit GameVariant(const QString& name, GameVariantCollection* collection=0);
	virtual ~GameVariant() {}

public:
	QString name() const { return m_name; }
	QString description() const { return m_description; }
	void setDescription(const QString& descr);
	QString icon() const { return m_icon; }
	void setIcon(const QString& icon);

	/// This method returs whether the variant has an configure option
	virtual bool canConfigure() const = 0;

	/// Shows a configure dialog and changes the settings
	virtual bool configure() = 0;

	/// Whether this variant can be started without any values in the grid
	virtual bool canStartEmpty() const = 0;

	/// Creates a game without a puzzle but with an empty grid
	virtual Game startEmpty() = 0;

	/// Creates an instance of this game variant
	virtual Game createGame(int difficulty, int symmetry) = 0;

	/// Creates the correct view for the game.
	/// Game needs to be compatible with this GameVariant
	virtual KsView* createView(const Game& game) const = 0;

private:
	QString m_name;
	QString m_description;
	QString m_icon;
};

class GameVariantCollection : public QAbstractListModel {
friend class GameVariant;
Q_OBJECT
public:
	GameVariantCollection(QObject* parent, bool autoDel);
	~GameVariantCollection();

public:
	void addVariant(GameVariant* variant);

public:
	QVariant data(const QModelIndex &index, int role) const override;
	int rowCount(const QModelIndex&) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	GameVariant* variant(const QModelIndex&) const;

Q_SIGNALS:
	void newVariant(GameVariant* variant);

public:
	QList<GameVariant*> m_variants;
	bool m_autoDelete;
};

class GameVariantDelegate : public QItemDelegate {
Q_OBJECT
public:
	enum Roles {
		Description = 33
	};

public:
	explicit GameVariantDelegate(QObject* parent = 0, QWidget * viewport = 0);
public:
	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

public:
	QString title(const QModelIndex& index) const;
	QString description(const QModelIndex& index) const;
	QString icon(const QModelIndex& index) const;
	bool configurable(const QModelIndex& index) const;
private:
	QWidget * m_viewport;

	static const int m_iconWidth = 48;
	static const int m_iconHeight = 48;
	static const int m_leftMargin = 16;
	static const int m_rightMargin = 12;
	static const int m_separatorPixels = 8;
	static const int m_minWidth = m_leftMargin + m_rightMargin +
				      4 * (m_iconWidth + m_separatorPixels*2);
};

class SudokuGame : public GameVariant {
public:
	SudokuGame(const QString& name, uint order, GameVariantCollection* collection=0);
	~SudokuGame();

public:
	bool canConfigure() const override;
	bool configure() override;
	bool canStartEmpty() const override;
	Game startEmpty() override;
	Game createGame(int difficulty, int symmetry) override;
	KsView* createView(const Game& game) const override;

private:
	uint m_order;
	uint m_symmetry;

	SKGraph *m_graph;
};

class RoxdokuGame : public GameVariant {
public:
	RoxdokuGame(const QString& name, uint order, GameVariantCollection* collection=0);
	~RoxdokuGame();

public:
	bool canConfigure() const override;
	bool configure() override;
	bool canStartEmpty() const override;
	Game startEmpty() override;
	Game createGame(int difficulty, int symmetry) override;
	KsView* createView(const Game& game) const override;

private:
	uint m_order;
	uint m_symmetry;

	SKGraph* m_graph;
};

class CustomGame : public GameVariant {
public:
	CustomGame(const QString& name, const QUrl& url, GameVariantCollection* collection=0);
	~CustomGame();

public:
	bool canConfigure() const override;
	bool configure() override;
	bool canStartEmpty() const override;
	Game startEmpty() override;
	Game createGame(int difficulty, int symmetry) override;
	KsView* createView(const Game& game) const override;

private:
	uint m_order;
	uint m_symmetry;

	QUrl m_url;
	SKGraph* m_graph;
	bool createSKGraphObject();
};

}

#endif
