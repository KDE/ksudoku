#ifndef BOARDCLASS_H
#define BOARDCLASS_H

#include <QScriptClass>

class QScriptContext;
class BoardWrapper;

class BoardClass : public QObject, public QScriptClass {
	Q_OBJECT
public:
	BoardClass(QScriptEngine *engine);
public:
	static QScriptValue constructorImplementation(QScriptContext *context, QScriptEngine *engine);
	QScriptValue constructor() const;
	QScriptValue prototype() const;
	QString name() const;
public:
	static BoardWrapper *valueToWrapper(const QScriptValue &value);
private:
	QScriptValue m_prototype;
	QScriptValue m_constructor;
};

#endif // BOARDCLASS_H
