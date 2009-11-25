#include "./itemclass.h"

#include <QScriptEngine>

#include "item.h"

#include "itemprototype.h"
#include "itemwrapper.h"

#include "objectmanager.h"

Q_DECLARE_METATYPE(ItemClass*);

ItemClass::ItemClass(QScriptEngine *engine, ObjectManager *objectManager)
	: QObject(engine), QScriptClass(engine)
{
	m_objectManager = objectManager;
	m_prototype = engine->newQObject(new ItemPrototype(engine), QScriptEngine::QtOwnership,
		QScriptEngine::SkipMethodsInEnumeration | QScriptEngine::ExcludeSuperClassMethods
		| QScriptEngine::ExcludeSuperClassProperties);
	QScriptValue global = engine->globalObject();
	m_prototype.setPrototype(global.property("Object").property("prototype"));

	m_constructor = engine->newFunction(constructorImplementation, 4);
	m_constructor.setData(engine->newVariant(QVariant::fromValue(this)));

}

QScriptValue ItemClass::constructorImplementation(QScriptContext* context, QScriptEngine* engine) {
	ItemClass *cls = context->callee().data().toVariant().value<ItemClass*>();
	if(!cls) return context->throwError(QScriptContext::UnknownError, "Item(): Internal data of constructor invalid");

	if(context->argumentCount() != 1)
		return context->throwError("Item(): requires exactly one argument");

	if(!context->argument(0).isString()) {
		return context->throwError(QScriptContext::TypeError, "Item(): first argument is not a string");
	}

	QString type = context->argument(0).toString();
	ItemWrapper *item = cls->m_objectManager->createItem(type, engine);

	if(!item) {
		return context->throwError(QString("Item(): first argument (\"%1\") not a valid item-type").arg(type));
	}

	return engine->newObject(cls, engine->newVariant(QVariant::fromValue(item)));
}

QScriptValue ItemClass::constructor() const {
	return m_constructor;
}

QScriptClass::QueryFlags ItemClass::queryProperty(const QScriptValue &object, const QScriptString &name,
	QueryFlags flags, unsigned int *id)
{
	ItemWrapper *wrapper = valueToWrapper(object);
	
	if(!wrapper || !wrapper->hasProperty(name)) return 0;
	
	return HandlesReadAccess | HandlesWriteAccess;
}

QScriptValue ItemClass::property(const QScriptValue &object, const QScriptString &name, unsigned int id) {
	ItemWrapper *wrapper = valueToWrapper(object);
	
	if(!wrapper) return QScriptClass::property(object, name, id);

	return wrapper->property(name);
}

void ItemClass::setProperty(QScriptValue &object, const QScriptString &name, unsigned int id, const QScriptValue &value) {
	ItemWrapper *wrapper = valueToWrapper(object);
	
	if(!wrapper || wrapper->isInitialized()) return;

	wrapper->setProperty(name, value);
}

QScriptValue::PropertyFlags ItemClass::propertyFlags(const QScriptValue &object, const QScriptString &name, unsigned int id) {
	ItemWrapper *wrapper = valueToWrapper(object);
	
	if(!wrapper) return QScriptClass::propertyFlags(object, name, id);
	
	return wrapper->propertyFlags(name);
}

QScriptValue ItemClass::prototype() const {
	return m_prototype;
}

QString ItemClass::name() const {
	return "logine.Item";
}

ItemWrapper *ItemClass::valueToWrapper(const QScriptValue& value) {
	return value.data().toVariant().value<ItemWrapper*>();
}
