#include "gameseldlg.h" 

#include <qlayout.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QVBoxLayout>
#include <klocale.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <kmessagebox.h>
#include <qsignalmapper.h>
#include <stdio.h> // TODO rem this
#include <kdialog.h>

namespace ksudoku {


GameSelectionButton::GameSelectionButton(const QString& text, QWidget* parent, const QString& id)
	: QPushButton(text, parent), m_id(id), m_group(0)
{ }

inline GameSelectionGroup* GameSelectionButton::group() const {
	return dynamic_cast<GameSelectionGroup*>(parent());
}


GameSelectionGroup::GameSelectionGroup(const QString& text, QWidget* parent)
	: QWidget(parent), m_text(text), m_gridLayout(0), m_mainLayout(new QVBoxLayout(this)), m_usedCols(1), m_idealCols(1)
{
	m_mainLayout->addWidget (new QLabel("<b>"+text+"</b>", this));
	
	m_gridLayout = new QGridLayout(); //TODO PORT
	m_gridLayout->setSpacing(10);
	m_gridLayout->setMargin(10);

	m_mainLayout->addLayout(m_gridLayout);
}

GameSelectionGroup::~GameSelectionGroup() {
	qDeleteAll(m_buttons);
	m_buttons.clear();
}

GameSelectionButton* GameSelectionGroup::addButton(const QString& id, const QString& text) {
	GameSelectionButton* button = new GameSelectionButton(text, this, id);
	m_buttons.append(button);
	updateColumnCount();
	updateLayout();
	return button;
}

bool GameSelectionGroup::removeButton(const QString& id) {
	GameSelectionButton *button;
// 	 for(button = m_buttons.first(); button; button = m_buttons.next()) {
	QMutableListIterator<GameSelectionButton *> it(m_buttons);
	while (it.hasNext()) {
		button = it.next();
		if(button->id() == id) {
			delete button;
			it.remove();

			updateColumnCount();
			updateLayout();
			return true;
		}
	}
	return false;
}

void GameSelectionGroup::updateColumnCount() {
	uint idealCols = m_buttons.count();
	if(idealCols == 0) idealCols = 1;
	if(idealCols > 4) idealCols = 4;
	
	if(idealCols != m_idealCols) {
		m_idealCols = idealCols;
		emit idealColumnCountChanged(idealCols);
	}
}

void GameSelectionGroup::setColumns(uint cols) {
	if(cols == m_usedCols) return;
		
	if(cols < m_usedCols) {
		/*delete layout();
		m_gridLayout = new QGridLayout(this, 1, cols); //TODO PORT m_mainLayout
		m_gridLayout->setSpacing(10);
		m_gridLayout->setMargin(10);
	        setLayout(m_gridLayout);*/
	}
	
	m_usedCols = cols;

	updateLayout();
}

void GameSelectionGroup::updateLayout() {
	//m_gridLayout->expand(1, 4); //TODO PORT
	uint pos = 0;

	QListIterator<GameSelectionButton*> it(m_buttons);
	while(it.hasNext()) {
		m_gridLayout->addWidget(it.next(), pos / m_usedCols, pos % m_usedCols);
		pos++;
	}
}

GameSelectionDialog::GameSelectionDialog(QWidget* parent) : QWidget(parent) {
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setSpacing(KDialog::spacingHint());
	m_mainLayout->setMargin(KDialog::marginHint());
	
	m_signalMapper = new QSignalMapper(this);
	connect(m_signalMapper, SIGNAL(mapped(const QString&)), this, SLOT(onButtonPressed(const QString&)));
	
	m_mainLayout->addWidget (new QLabel("<center><h1>"+i18n("Welcome to KSudoku")+"</h1></center>", this));		
}

GameSelectionDialog::~GameSelectionDialog() {
	qDeleteAll(m_groups);
	m_groups.clear();
}

void GameSelectionDialog::addEntry(const QString& name, const QString& text, const QString& groupTitle) {
	GameSelectionGroup* group = 0;
	QListIterator<GameSelectionGroup*> it(m_groups);
	while(it.hasNext()) {
		group = it.next();
		if(group->text() == groupTitle) break;
	}
	if(!group) {
		group = new GameSelectionGroup(groupTitle, this);
		m_mainLayout->addWidget(group);
		m_groups.append(group);
		connect(group, SIGNAL(idealColumnCountChanged(uint)), this, SLOT(updateColumnCount()));
		connect(this, SIGNAL(columnCountChanged(uint)), group, SLOT(setColumns(uint)));
	}
	GameSelectionButton* button = group->addButton(name, text);
	button->show();
	m_signalMapper->setMapping(button, name);
	connect(button, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
}

bool GameSelectionDialog::removeEntry(const QString& name) {
	QListIterator<GameSelectionGroup*> it(m_groups);
	while(it.hasNext()) {
		if(it.next()->removeButton(name))
			return true;
	}
	return false;
}

void GameSelectionDialog::onButtonPressed(const QString& name) {
	emit gameSelected(name);
}

void GameSelectionDialog::UPDATE()
{
	updateColumnCount();
}

void GameSelectionDialog::updateColumnCount() {
	int columns = 1;
	QListIterator<GameSelectionGroup*> it(m_groups);
	while(it.hasNext()) {
		int groupColumns = it.next()->idealColumns();
		if(groupColumns > columns)
			columns = groupColumns;
	}
	emit columnCountChanged(columns);
}

}

#include "gameseldlg.moc"
