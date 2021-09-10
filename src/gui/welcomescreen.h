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

#ifndef _KSUDOKUWELCOMESCREEN_H_
#define _KSUDOKUWELCOMESCREEN_H_

#include <QWidget>

#include "ui_welcomescreen.h"
#include "gamevariants.h"

namespace ksudoku {

class WelcomeScreen : public QFrame, private Ui::WelcomeScreen {
Q_OBJECT

public:
	WelcomeScreen(QWidget* parent, GameVariantCollection* collection);
	
public:
	GameVariant* selectedVariant() const;

	int difficulty() const;
	void setDifficulty(int difficulty);
	int symmetry() const;
	void setSymmetry(int symmetry);

private Q_SLOTS:
	void onCurrentVariantChange();
	
	void getNewVariant();
	void configureVariant();
	void startEmptyGame();
	void playVariant();

	void generatePuzzle();

	// This slot is used to restore the user's previous selection of puzzle
	// type and size when KSudoku starts up.  It has to be a slot because it
	// needs a queued call, which executes after all the setup has finished,
	// otherwise selection fails and no selection is shown.
	void setSelectedVariant(int row);

Q_SIGNALS:
	void newGameStarted(const ::ksudoku::Game& game, GameVariant* variant);
	
private:
	GameVariantCollection* m_collection;
	int m_selectedPuzzle;
	int m_difficulty;
	int m_symmetry;
};

}

#endif
