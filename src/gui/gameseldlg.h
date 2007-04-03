#ifndef _KSUDOKUGAMESELECTIONDIALOG_H_
#define _KSUDOKUGAMESELECTIONDIALOG_H_

#include <qmap.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
//#include <qhbuttongroup.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <Q3PtrList>
#include <QLabel>

class QPushButton;
class QSignalMapper;
class QGridLayout;
class QVBoxLayout;
class QLabel;
namespace ksudoku {

class GameSelectionGroup;
class GameSelectionButton : public QPushButton {
Q_OBJECT
public:
	GameSelectionButton(const QString& text, QWidget* parent, const QString& id);
public:
	inline QString id() const { return m_id; }
	inline GameSelectionGroup* group() const;
private:
	QString m_id;
	GameSelectionGroup* m_group;
};

class GameSelectionGroup : public QWidget {
Q_OBJECT
public:
	GameSelectionGroup(const QString& text, QWidget* parent);
	~GameSelectionGroup();
	
	GameSelectionButton* addButton(const QString& id, const QString& text);
	bool removeButton(const QString& id);
	
	inline QString text() const { return m_text; }
	inline uint idealColumns() const { return m_idealCols; }


	
public slots:
	void setColumns(uint cols);
	
private:
	void updateColumnCount();
	void updateLayout();
	
signals:
	void idealColumnCountChanged(uint count);
	
private:
	QString m_text;
	QGridLayout* m_gridLayout;
	QVBoxLayout* m_mainLayout;
	QList<GameSelectionButton*> m_buttons;
	uint m_usedCols;
	uint m_idealCols;
};

class GameSelectionDialog : public QWidget {
	Q_OBJECT
	public:
		GameSelectionDialog(QWidget* parent);
		~GameSelectionDialog();
		
		void UPDATE();
// 		void showOptions();
		void addEntry(const QString& name, const QString& text, const QString& groupTitle = QString());
		bool removeEntry(const QString& name);
	
	signals:
		void gameSelected(const QString& name);
		void columnCountChanged(uint cols);
		
	private slots: 
		void onButtonPressed(const QString& name);
		void updateColumnCount();
	
	private:
		QVBoxLayout* m_mainLayout;
		QSignalMapper* m_signalMapper;
		QList<GameSelectionGroup*> m_groups;
};

}

#endif
