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

#include <QDebug>

#include <KConfigGroup>
#include <KMessageBox>
#include <KSharedConfig>

#include "ksudokugame.h"
#include "globals.h"
#include "puzzle.h"

Q_DECLARE_METATYPE(ksudoku::GameVariant*)

namespace ksudoku {

WelcomeScreen::WelcomeScreen(QWidget* parent, GameVariantCollection* collection)
	: QFrame(parent), m_collection(collection)
{
	setupUi(this);	// Get gameListWidget by loading from welcomescreen.ui.

	// Set the screen to display a multi-column list of puzzle-types, with
	// vertical scrolling.  GameVariantDelegate::sizeHint() calculates the
	// number of columns and their width when it works out the size of the
	// item's display-area.

	QItemDelegate* delegate =
		new GameVariantDelegate(this, gameListWidget->viewport());
	gameListWidget->setWrapping(true);
	gameListWidget->setResizeMode(QListView::Adjust);
	gameListWidget->setUniformItemSizes(true);
	gameListWidget->setFlow(QListView::LeftToRight);

	// Avoid a resize loop (with the scrollbar appearing and disappearing)
	// if ever the number of items and display-columns hits a bad combo.
	gameListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	gameListWidget->setModel(m_collection);
	gameListWidget->setItemDelegate(delegate);
	gameListWidget->setVerticalScrollMode(QListView::ScrollPerPixel);
	gameListWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	gameListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	// Get the previous puzzle configuration.
	KConfigGroup gameGroup (KSharedConfig::openConfig(), "KSudokuGame");
	m_selectedPuzzle = gameGroup.readEntry("SelectedPuzzle", 0);
	m_difficulty     = gameGroup.readEntry("Difficulty", (int) VeryEasy);
	m_symmetry       = gameGroup.readEntry("Symmetry"  , (int) CENTRAL);

	// This has to be a deferred call (presumably to allow the view's setup
	// to complete), otherwise the selection fails to appear.
	QMetaObject::invokeMethod (this, "setSelectedVariant",
		                   Qt::QueuedConnection,
				   Q_ARG (int, m_selectedPuzzle));

	connect(gameListWidget->selectionModel(), &QItemSelectionModel::currentChanged, this, &WelcomeScreen::onCurrentVariantChange);
	
	connect(getNewGameButton, &QPushButton::clicked, this, &WelcomeScreen::getNewVariant);
	// TODO disabled due to missing per-game config dialog
// 	connect(configureGameButton, SIGNAL(clicked(bool)), this, SLOT(configureVariant()));
	// connect(playGameButton, SIGNAL(clicked(bool)), this, SLOT(playVariant()));			// Disable old create-game code.
	// connect(gameListWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(playVariant()));	// Disable old create-game code.

	connect(startEmptyButton, &QPushButton::clicked, this, &WelcomeScreen::startEmptyGame);
	connect(puzzleGeneratorButton, &QPushButton::clicked, this, &WelcomeScreen::generatePuzzle);
	connect(gameListWidget, &QListView::doubleClicked, this, &WelcomeScreen::generatePuzzle);

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
	KMessageBox::information(this, i18n("GetNewVariant not implemented"), QLatin1String(""));
}

void WelcomeScreen::configureVariant() {
	GameVariant* variant = selectedVariant();
	if(!variant) return;
	
	variant->configure();
}

void WelcomeScreen::startEmptyGame() {
	GameVariant* variant = selectedVariant();
	if(!variant) {
		KMessageBox::sorry(this, i18n("Please select a puzzle variant."), i18n("Unable to start puzzle"));
		return;
	}
	
	Game game = variant->startEmpty();
	if (! game.isValid()) {
		KMessageBox::sorry(this, i18n("Unable to create an empty puzzle of the chosen variant; please try another."), i18n("Unable to start puzzle"));
		return;
	}
	
	emit newGameStarted(game, variant);
}

void WelcomeScreen::playVariant() {
	return;		// Disable old game-creation code.
	GameVariant* variant = selectedVariant();
	if(!variant) {
		KMessageBox::sorry(this, i18n("Please select a puzzle variant."), i18n("Unable to start puzzle"));
		return;
	}
	
	Game game = variant->createGame(difficulty(), 0);
	if(!game.isValid()) {
		KMessageBox::sorry(this, i18n("Unable to start a puzzle of the chosen variant; please try another."), i18n("Unable to start puzzle"));
		return;
	}

	emit newGameStarted(game, variant);
}

void WelcomeScreen::generatePuzzle() {
	GameVariant* variant = selectedVariant();
	if(!variant) {
		KMessageBox::sorry(this, i18n("Please select a puzzle variant."), i18n("Unable to start puzzle"));
		return;
	}

	Game game = variant->createGame(difficulty(), symmetry());
	if(!game.isValid()) {
		KMessageBox::sorry(this, i18n("Unable to generate a puzzle of the chosen variant; please try another."), i18n("Unable to start puzzle"));
		return;
	}

	// Save the selected puzzle configuration.
	QModelIndex index = gameListWidget->currentIndex();
	m_selectedPuzzle = index.row();

	KConfigGroup gameGroup (KSharedConfig::openConfig(), "KSudokuGame");
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


