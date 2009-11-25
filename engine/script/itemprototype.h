#ifndef ITEMPROTOTYPE_H
#define ITEMPROTOTYPE_H

#include <QObject>
#include <QScriptable>

class ItemWrapper;

class ItemPrototype : public QObject, public QScriptable {
	Q_OBJECT
public:
	ItemPrototype(QObject *parent);
public slots:
	QString toString() const;
private:
	ItemWrapper *thisItem() const;
};

#endif // ITEMPROTOTYPE_H
