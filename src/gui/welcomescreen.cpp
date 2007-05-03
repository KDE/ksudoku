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
#include "welcomescreen.h"

#include <QListWidget>
#include <QVBoxLayout>
#include <QtDebug>
#include <KMessageBox>

#include "ksudokugame.h"

Q_DECLARE_METATYPE(ksudoku::GameVariant*)

namespace ksudoku {

WelcomeScreen::WelcomeScreen(QWidget* parent, GameVariantCollection* collection)
	: QFrame(parent), m_collection(collection)
{
	connect(collection, SIGNAL(newVariant(GameVariant*)), this, SLOT(onNewVariant(GameVariant*)));
	
	setupUi(this);
	connect(getNewGameButton, SIGNAL(clicked(bool)), this, SLOT(getNewVariant()));
	connect(configureGameButton, SIGNAL(clicked(bool)), this, SLOT(configureVariant()));
	connect(startEmptyButton, SIGNAL(clicked(bool)), this, SLOT(startEmptyGame()));
	connect(playGameButton, SIGNAL(clicked(bool)), this, SLOT(playVariant()));
	
	connect(gameListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(onCurrentVariantChange()));
}

GameVariant* WelcomeScreen::selectedVariant() const {
	QListWidgetItem* item = gameListWidget->currentItem();
	if(!item) return 0;
	return qvariant_cast<GameVariant*>(item->data(Qt::UserRole));
}

int WelcomeScreen::difficulty() const {
	return difficultySlider->value();
}

void WelcomeScreen::setDifficulty(int difficulty) {
	difficultySlider->setValue(difficulty);
}

void WelcomeScreen::onNewVariant(GameVariant* variant) {
	QListWidgetItem* item = new QListWidgetItem(variant->name(), gameListWidget);
	item->setData(Qt::UserRole, qVariantFromValue(variant));
}

void WelcomeScreen::onCurrentVariantChange() {
	GameVariant* variant = selectedVariant();
	if(!variant) {
		configureGameButton->setEnabled(false);
		playGameButton->setEnabled(false);
		return;
	}
	
	configureGameButton->setEnabled(variant->canConfigure());
	startEmptyButton->setEnabled(variant->canStartEmpty());
	playGameButton->setEnabled(true);
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
	
	emit newGameStarted(game, variant);
}

void WelcomeScreen::playVariant() {
	GameVariant* variant = selectedVariant();
	if(!variant) return;
	
	Game game = variant->createGame(difficulty());
	
	emit newGameStarted(game, variant);
}

}

#include "welcomescreen.moc"
