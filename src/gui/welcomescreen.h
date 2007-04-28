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

private slots:
	void onNewVariant(GameVariant* variant);
	
	void onCurrentVariantChange();
	
	void getNewVariant();
	void createNewVariant();
	void configureVariant();
	void playVariant();
	
signals:
	void newGameStarted(const Game& game, GameVariant* variant);
	
private:
	GameVariantCollection* m_collection;
};

}

#endif
