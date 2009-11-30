#include "boardclass.h"

#include <QScriptEngine>

#include "item.h"

#include "boardprototype.h"
#include "boardwrapper.h"

Q_DECLARE_METATYPE(BoardClass*)

BoardClass::BoardClass(QScriptEngine *engine)
	: QObject(engine), QScriptClass(engine)
{
	m_prototype = engine->newQObject(new BoardPrototype(engine), QScriptEngine::QtOwnership,
		QScriptEngine::SkipMethodsInEnumeration | QScriptEngine::ExcludeSuperClassMethods
		| QScriptEngine::ExcludeSuperClassProperties);
	QScriptValue global = engine->globalObject();
	m_prototype.setPrototype(global.property("Object").property("prototype"));

	m_constructor = engine->newFunction(constructorImplementation, 4);
	m_constructor.setData(engine->newVariant(QVariant::fromValue(this)));

}

QScriptValue BoardClass::constructorImplementation(QScriptContext* context, QScriptEngine* engine) {
	BoardClass *cls = context->callee().data().toVariant().value<BoardClass*>();
	if(!cls) return context->throwError(QScriptContext::UnknownError, "Board(): Internal data of constructor invalid");

	int x = 1, y = 1, z = 1, w = 1;
	int args = context->argumentCount();
	if(args > 4) args = 4;
	switch(args) {
		case 4: w = context->argument(3).toInt32();
		case 3: z = context->argument(2).toInt32();
		case 2: y = context->argument(1).toInt32();
		case 1: x = context->argument(0).toInt32();
			break;
		case 0:
			return context->throwError("Board(x,y,z,w): requires at least one argument.");
	}
	if(x <= 0)
		return context->throwError(QScriptContext::RangeError, "Board(x,y,z,w): The x-size has to be at least 1.");
	if(y <= 0)
		return context->throwError(QScriptContext::RangeError, "Board(x,y,z,w): The y-size has to be at least 1.");
	if(z <= 0)
		return context->throwError(QScriptContext::RangeError, "Board(x,y,z,w): The z-size has to be at least 1.");
	if(w <= 0)
		return context->throwError(QScriptContext::RangeError, "Board(x,y,z,w): The w-size has to be at least 1.");

	ItemBoard *board = new ItemBoard(x, y, z, w);
	BoardWrapper *wrapper = new BoardWrapper(board);

	return engine->newObject(cls, engine->newVariant(QVariant::fromValue(wrapper)));
}

QScriptValue BoardClass::constructor() const {
	return m_constructor;
}

QScriptValue BoardClass::prototype() const {
	return m_prototype;
}

QString BoardClass::name() const {
	return "logine.Board";
}

BoardWrapper* BoardClass::valueToWrapper(const QScriptValue& value) {
	return value.data().toVariant().value<BoardWrapper*>();
}
