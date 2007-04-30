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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _KSUDOKUGAMEVARIANTS_H_
#define _KSUDOKUGAMEVARIANTS_H_

#include <QObject>
#include <QList>
#include <KUrl>

class SKSolver;
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
	
	/// This method returs whether the variant has an configure option
	virtual bool canConfigure() const = 0;
	
	/// Shows a configure dialog and changes the settings
	virtual bool configure() = 0;
	
	/// Creates a instance of this game variant
	virtual Game createGame(int difficulty) const = 0;
	
	/// Creates the correct view for the game.
	/// Game needs to be compatible with this GameVariant
	virtual KsView* createView(const Game& game) const = 0;

private:
	QString m_name;
};

class GameVariantCollection : public QObject {
friend class GameVariant;
Q_OBJECT
public:
	GameVariantCollection(QObject* parent, bool autoDel);
	~GameVariantCollection();
		
public:
	void addVariant(GameVariant* variant);

signals:
	void newVariant(GameVariant* variant);
	
public:
	QList<GameVariant*> m_variants;
	bool m_autoDelete;
};

class SudokuGame : public GameVariant {
public:
	SudokuGame(const QString& name, uint order, GameVariantCollection* collection=0);
	
public:
	bool canConfigure() const;
	bool configure();
	Game createGame(int difficulty) const;
	KsView* createView(const Game& game) const;
	
private:
	uint m_order;
	uint m_symmetry;
	
	mutable SKSolver* m_solver;
};

class RoxdokuGame : public GameVariant {
public:
	RoxdokuGame(const QString& name, uint order, GameVariantCollection* collection=0);
	
public:
	bool canConfigure() const;
	bool configure();
	Game createGame(int difficulty) const;
	KsView* createView(const Game& game) const;
	
private:
	uint m_order;
	uint m_symmetry;
	
	mutable SKSolver* m_solver;
};

class CustomGame : public GameVariant {
public:
	CustomGame(const QString& name, const KUrl& url, GameVariantCollection* collection=0);
	
public:
	bool canConfigure() const;
	bool configure();
	Game createGame(int difficulty) const;
	KsView* createView(const Game& game) const;
	
private:
	KUrl m_url;
	mutable SKSolver* m_solver;
};

}

#endif
