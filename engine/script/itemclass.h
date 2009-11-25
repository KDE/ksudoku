#ifndef ITEMCLASS_H
#define ITEMCLASS_H

#include <QScriptClass>

class QScriptContext;
class ObjectManager;

class Item;
class ItemWrapper;

class ItemClass : public QObject, public QScriptClass {
	Q_OBJECT
public:
	ItemClass(QScriptEngine *engine, ObjectManager *manager);
public:
	static QScriptValue constructorImplementation(QScriptContext *context, QScriptEngine *engine);
	QScriptValue constructor() const;
	QueryFlags queryProperty(const QScriptValue&, const QScriptString&, QueryFlags, unsigned int*);
	QScriptValue property(const QScriptValue&, const QScriptString&, unsigned int);
	void setProperty(QScriptValue&, const QScriptString&, unsigned int, const QScriptValue&);
	QScriptValue::PropertyFlags propertyFlags(const QScriptValue&, const QScriptString&, unsigned int);
	QScriptValue prototype() const;
	QString name() const;
public:
	static ItemWrapper *valueToWrapper(const QScriptValue &value);
private:
	ObjectManager *m_objectManager;
	QScriptValue m_prototype;
	QScriptValue m_constructor;
};

#endif // ITEMCLASS_H
