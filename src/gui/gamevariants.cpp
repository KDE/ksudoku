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

///////////////////////////////////////////////////////////////////////////////
// class GameVariantCollection
///////////////////////////////////////////////////////////////////////////////

GameVariantCollection::GameVariantCollection(QObject* parent, bool autoDel)
	: QObject(parent), m_autoDelete(autoDel)
{
}

GameVariantCollection::~GameVariantCollection() {
	if(m_autoDelete)
		qDeleteAll(m_variants);
	m_variants.clear();
}

void GameVariantCollection::addVariant(GameVariant* variant) {
	m_variants.append(variant);
	emit newVariant(variant);
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