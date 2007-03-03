// part of KSUDOKU - by Francesco Rossi <redsh@email.it> 2005
#ifndef KNEWDLG_H
#define KNEWDLG_H

#include <kdialog.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <QGroupBox>
/**
@author Francesco Rossi
*/

class KConfig;

namespace ksudoku {

class GameOptionsDialog : public QWidget
{
Q_OBJECT
public:
	GameOptionsDialog(QWidget* parent, bool dub=false, int type=0, int order=9);
	~GameOptionsDialog();

public:
	uint symmetry() const;
	int difficulty() const;
	uint order() const;
	uint type() const;
	QString shapeName() const;

	void setSymmetry(int sym);
	void setDifficulty(int diff);
	void setOrder(int order);
	void setType(uint type);
	void setShapeName(const QString& name);

	///get settings from global session or use defaults
	void readSettings()  ;
	///write settings to global session (not to file)
	void writeSettings();

private:
	QGroupBox *btnsDifficulty;
	QGroupBox *btnsOrder;
	QGroupBox *btnsType;
	QGroupBox *btnssymmetry;

	QRadioButton  *rbType[3];
	QRadioButton  *rbDifficulty[4];
	QRadioButton  *rbOrder[4];
	QRadioButton  *rbsymmetry[5];
	
	QString m_shapeName;

	///reference to session config
	KConfig& m_config;
};

}

#endif
