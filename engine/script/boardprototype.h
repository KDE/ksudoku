#ifndef BOARDPROTOTYPE_H
#define BOARDPROTOTYPE_H

#include <QObject>
#include <QScriptable>
#include <QVector>
#include <QScriptValue>
#include <QVariant>

class BoardWrapper;

class BoardPrototype : public QObject, public QScriptable {
	Q_OBJECT
public:
	BoardPrototype(QObject *parent);
public slots:
	void setItem(const QScriptValue &item, int x = 0, int y = 0, int z = 0, int w = 0);
	QScriptValue items(int x, int y = -1, int z = -1, int w = -1);
	QScriptValue split(int x, int y = 0, int z = 0, int w = 0);
private:
	BoardWrapper *thisBoard() const;
};

#endif // BOARDPROTOTYPE_H
