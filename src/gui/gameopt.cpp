// part of KSUDOKU - by Francesco Rossi <redsh@email.it> 2005
#include "gameopt.h"
#include <qlayout.h>
#include <QGridLayout>

//#include <kapp.h>
#include <kdialog.h>
#include <kmessagebox.h>
#include "roxdokuview.h"
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>

#include "gameopt.moc"

namespace ksudoku {

GameOptionsDialog::GameOptionsDialog(QWidget* parent, bool dub, int type, int order)
	: QWidget(parent)
	, m_config(*KApplication::kApplication()->sessionConfig())
{
	QGridLayout *top_layout = new QGridLayout(this);//,2,2,5);
        parent->layout()->addWidget(this);
	//top_layout->addColSpacing (20,5); //TODO PORT
	//top_layout->addRowSpacing (20,20);
	//top_layout->setAutoAdd(true);
	QVBoxLayout *vbox;

	btnsType       = new QGroupBox(i18n("Dimensions") , this);
	btnsOrder      = new QGroupBox(i18n("Order"     ) , this);
	btnsDifficulty = new QGroupBox(i18n("Difficulty") , this);
	btnssymmetry   = new QGroupBox(i18n("symmetry"  ) , this);
        top_layout->addWidget(btnsType,0,0);
	top_layout->addWidget(btnsOrder,0,1);
	top_layout->addWidget(btnsDifficulty,1,0);
	top_layout->addWidget(btnssymmetry,1,1);

	rbType[0] = new QRadioButton(i18n("2D    (Sudoku)" ));//, "2D (Sudoku)"    );
	rbType[1] = new QRadioButton(i18n("3D (Roxdoku)"));//btnsType, "3D (Roxdoku)");
	rbType[2] = new QRadioButton(i18n("Custom: -None-"));//, "Custom-shaped");
	rbType[2]->setEnabled(false);
        vbox = new QVBoxLayout;
        vbox->addWidget(rbType[0]);
        vbox->addWidget(rbType[1]);
        vbox->addWidget(rbType[2]);
        vbox->addStretch(1);
        btnsType->setLayout(vbox);


	rbType[type]->setChecked(true);

	rbOrder[0] = new QRadioButton(i18n( "9"));//,  "9");
	rbOrder[1] = new QRadioButton(i18n("16"));//, "16");
	rbOrder[2] = new QRadioButton(i18n("25"));//, "25");
	rbOrder[0]->setChecked(true);
        vbox = new QVBoxLayout;
        vbox->addWidget(rbOrder[0]);
        vbox->addWidget(rbOrder[1]);
        vbox->addWidget(rbOrder[2]);
        vbox->addStretch(1);
        btnsOrder->setLayout(vbox);
	
	rbDifficulty[0] = new QRadioButton(i18n("Easy"   ));//, btnsDifficulty, "easy"   );
	rbDifficulty[1] = new QRadioButton(i18n("Medium" ));//, btnsDifficulty, "medium" );
	rbDifficulty[2] = new QRadioButton(i18n("Hard"   ));//, btnsDifficulty, "hard"   );
	rbDifficulty[3] = new QRadioButton(i18n("Hardest"));//, btnsDifficulty, "hardest");
	rbDifficulty[1]->setChecked(true);
        vbox = new QVBoxLayout;
        vbox->addWidget(rbDifficulty[0]);
        vbox->addWidget(rbDifficulty[1]);
        vbox->addWidget(rbDifficulty[2]);
        vbox->addWidget(rbDifficulty[3]);
        vbox->addStretch(1);
        btnsDifficulty->setLayout(vbox);

	rbsymmetry[0] = new QRadioButton(i18n("Random"  ));//, btnssymmetry, "Random"  );
	rbsymmetry[1] = new QRadioButton(i18n("None"    ));//, btnssymmetry, "None"    );
	rbsymmetry[2] = new QRadioButton(i18n("Diagonal"));//, btnssymmetry, "Diagonal");
	rbsymmetry[3] = new QRadioButton(i18n("Central" ));//, btnssymmetry, "Central" );
	rbsymmetry[4] = new QRadioButton(i18n("4-way"   ));//, btnssymmetry, "4-way"   );
	rbsymmetry[0]->setChecked(true);
        vbox = new QVBoxLayout;
        vbox->addWidget(rbsymmetry[0]);
        vbox->addWidget(rbsymmetry[1]);
        vbox->addWidget(rbsymmetry[2]);
        vbox->addWidget(rbsymmetry[3]);
        vbox->addWidget(rbsymmetry[4]);
        vbox->addStretch(1);
        btnssymmetry->setLayout(vbox);

	readSettings();
	setType(type);
	setOrder(order);

	//if(dub) { //TODO PORT
		//btnssymmetry->setEnabled(false);
		//btnsDifficulty->setEnabled(false);
	//}

	//btnsDifficulty->show();
	//btnsOrder->show();	
}

GameOptionsDialog::~GameOptionsDialog()
{
	// No need to delete something, as everything is a child of this
}


uint GameOptionsDialog::symmetry() const {
	if(type() != 0)
		return 1;
	
	for(int i=0; i<5; ++i)
		if(rbsymmetry[i]->isChecked())
			return i;
	return 0;
}

int GameOptionsDialog::difficulty() const {
	for(int i=0; i<4; ++i)
		if(rbDifficulty[i]->isChecked())
			return 2-i; //level between ??
	return 0;
}

uint GameOptionsDialog::order() const {
	if(rbOrder[1]->isChecked())
		return 16;
	if(rbOrder[2]->isChecked())
		return 25;
	return 9;
}

uint GameOptionsDialog::type() const {
	if(rbType[1]->isChecked())
		return 1;
	if(rbType[2]->isChecked())
		return 2;
	return 0;
}

QString GameOptionsDialog::shapeName() const {
	return m_shapeName;
}

void GameOptionsDialog::setSymmetry(int sym) {
	if(sym < 0) {
		//btnssymmetry->setEnabled(false); //TODO PORT
	} else {
		rbsymmetry[sym]->setChecked(true);
	}
}

void GameOptionsDialog::setDifficulty(int diff) {
	if(diff < -1)
	{
		//btnssymmetry->setEnabled(false); //TODO PORT
	}	
	else if(diff > 2) {
		//btnssymmetry->setEnabled(true);//TODO PORT
		rbDifficulty[0]->setChecked(true);
	} else {
		//btnssymmetry->setEnabled(true);//TODO PORT
		rbDifficulty[2-diff]->setChecked(true);
	}
}

void GameOptionsDialog::setOrder(int order) {
	if(order < 0) {
		//btnsOrder->setEnabled(false);//TODO PORT
		return;
	}
	//btnsOrder->setEnabled(true);//TODO PORT
	int orderIndex = 0;
	switch(order){
		case  9:  orderIndex = 0; break;
		case 16:  orderIndex = 1; break;
		case 25:  orderIndex = 2; break;
	}
	rbOrder[orderIndex]->setChecked(true);
}

void GameOptionsDialog::setType(uint type) {
	rbType[type]->setChecked(true);
}

void GameOptionsDialog::setShapeName(const QString& name) {
	m_shapeName = name;
	if(name.isNull()) {
		rbType[2]->setText("Custom: -None-");
		rbType[2]->setEnabled(false);
	} else {
		rbType[2]->setText("Custom: "+name);
		rbType[2]->setEnabled(true);
	}
}

void GameOptionsDialog::readSettings()
{
	setDifficulty(m_config.readNumEntry("difficulty", 1));
	setType      (m_config.readNumEntry("type"      , 0));
	setSymmetry  (m_config.readNumEntry("Symmetry"  , 0));
	setOrder     (m_config.readNumEntry("order"     , 9));
}

void GameOptionsDialog::writeSettings()
{
// 	if(!m_success)
// 		return; //do not write settings
// 
	m_config.writeEntry("difficulty", difficulty());
	m_config.writeEntry("type"      , type()      );
	m_config.writeEntry("Symmetry"  , symmetry()  );
	m_config.writeEntry("order"     , order()     );
}


}
