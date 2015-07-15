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
#include "welcomescreen.h"

#include <KMessageBox>

#include <QDebug>

#include "ksudokugame.h"
#include "globals.h"
#include "puzzle.h"

Q_DECLARE_METATYPE(ksudoku::GameVariant*)

namespace ksudoku {

WelcomeScreen::WelcomeScreen(QWidget* parent, GameVariantCollection* collection)
	: QFrame(parent), m_collection(collection)
{
	QItemDelegate* delegate = new GameVariantDelegate(this);
	
	setupUi(this);
	gameListWidget->setModel(m_collection);
	gameListWidget->setItemDelegate(delegate);
	gameListWidget->setVerticalScrollMode(QListView::ScrollPerPixel);
	gameListWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	gameListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	// Get the previous puzzle configuration.
	KConfigGroup gameGroup (KGlobal::config(), "KSudokuGame");
	m_selectedPuzzle = gameGroup.readEntry("SelectedPuzzle", 0);
	m_difficulty     = gameGroup.readEntry("Difficulty", (int) VeryEasy);
	m_symmetry       = gameGroup.readEntry("Symmetry"  , (int) CENTRAL);

	// This has to be a deferred call (presumably to allow the view's setup
	// to complete), otherwise the selection fails to appear.
	QMetaObject::invokeMethod (this, "setSelectedVariant",
		                   Qt::QueuedConnection,
				   Q_ARG (int, m_selectedPuzzle));

	connect(gameListWidget->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onCurrentVariantChange()));
	
	connect(getNewGameButton, SIGNAL(clicked(bool)), this, SLOT(getNewVariant()));
	// TODO disabled due to missing per-game config dialog
// 	connect(configureGameButton, SIGNAL(clicked(bool)), this, SLOT(configureVariant()));
	// connect(playGameButton, SIGNAL(clicked(bool)), this, SLOT(playVariant()));			// Disable old create-game code.
	// connect(gameListWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(playVariant()));	// Disable old create-game code.

	connect(startEmptyButton, SIGNAL(clicked(bool)), this, SLOT(startEmptyGame()));
	connect(puzzleGeneratorButton, SIGNAL(clicked(bool)), this, SLOT(generatePuzzle()));
	connect(gameListWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(generatePuzzle()));

	// GHNS is not implemented yet, so don't show an unuseful button
	getNewGameButton->hide();
}

GameVariant* WelcomeScreen::selectedVariant() const {
	QModelIndex index = gameListWidget->currentIndex();
	return m_collection->variant(index);
}

void WelcomeScreen::setSelectedVariant(int row) {
	gameListWidget->setCurrentIndex(gameListWidget->model()->index(row,0));
}

int WelcomeScreen::difficulty() const {
	return m_difficulty;
}

void WelcomeScreen::setDifficulty(int difficulty) {
	m_difficulty = difficulty;
}

int WelcomeScreen::symmetry() const {
	return m_symmetry;
}

void WelcomeScreen::setSymmetry(int symmetry) {
	m_symmetry = symmetry;
}

void WelcomeScreen::onCurrentVariantChange() {
	GameVariant* variant = selectedVariant();
	if(!variant) {
		// TODO disabled due to missing per-game config dialog
// 		configureGameButton->setEnabled(false);
		// playGameButton->setEnabled(false);
		puzzleGeneratorButton->setEnabled(false);
		return;
	}
	
	// TODO disabled due to missing per-game config dialog
// 	configureGameButton->setEnabled(variant->canConfigure());
	startEmptyButton->setEnabled(variant->canStartEmpty());
	// playGameButton->setEnabled(true);
	puzzleGeneratorButton->setEnabled(true);
}

void WelcomeScreen::getNewVariant() {
	KMessageBox::information(this, i18n("GetNewVariant not implemented"), "");
}

void WelcomeScreen::configureVariant() {
	GameVariant* variant = selectedVariant();
	if(!variant) return;
	
	variant->configure();
}

void WelcomeScreen::startEmptyGame() {
	GameVariant* variant = selectedVariant();
	if(!variant) return;
	
	Game game = variant->startEmpty();
	if (! game.isValid()) return;
	
	emit newGameStarted(game, variant);
}

void WelcomeScreen::playVariant() {
	return;		// Disable old game-creation code.
	GameVariant* variant = selectedVariant();
	if(!variant) return;
	
	Game game = variant->createGame(difficulty(), 0);

	emit newGameStarted(game, variant);
}

void WelcomeScreen::generatePuzzle() {
	GameVariant* variant = selectedVariant();
	if(!variant) return;

	Game game = variant->createGame(difficulty(), symmetry());

	// Save the selected puzzle configuration.
	QModelIndex index = gameListWidget->currentIndex();
	m_selectedPuzzle = index.row();

	KConfigGroup gameGroup (KGlobal::config(), "KSudokuGame");
	gameGroup.writeEntry("SelectedPuzzle", m_selectedPuzzle);
	gameGroup.writeEntry("Difficulty", m_difficulty);
	gameGroup.writeEntry("Symmetry"  , m_symmetry);
	gameGroup.sync();		// Ensure that the entry goes to disk.

	// If the user abandoned puzzle-generation, stay on the Welcome Screen
	// and allow the user to change the Difficulty, etc. of the puzzle.
	if (game.puzzle()->hasSolution()) {
	    emit newGameStarted(game, variant);		// OK, start playing.
	}
}

}

#include "welcomescreen.moc"
