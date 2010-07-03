#include "./scriptedruleset.h"

#include <QScriptEngine>
#include <QStringList>
#include <QScriptValueIterator>

#include "objectmanager.h"

#include "boardclass.h"
#include "itemclass.h"
#include "boardwrapper.h"
#include "itemwrapper.h"

#include "item.h"

#include <QDebug>

Q_DECLARE_METATYPE(QScriptValue)
Q_DECLARE_METATYPE(QVector<QScriptValue>)

class ScriptedRulesetPrivate {
	Q_DECLARE_PUBLIC(ScriptedRuleset)
public:
	ScriptedRulesetPrivate(ScriptedRuleset *ruleset);
public:
	void initItemArray(const QScriptValue &value);
private:
	QScriptEngine engine;
	QScriptValue logine;
	QScriptValue ruleset;
	QScriptValue items;
	ObjectManager objectManager;
	BoardClass *logineBoardClass;
	ItemClass *logineItemClass;
protected:
	ScriptedRuleset *q_ptr;
};

ScriptedRulesetPrivate::ScriptedRulesetPrivate(ScriptedRuleset *ruleset) {
	q_ptr = ruleset;

	qScriptRegisterSequenceMetaType<QVector<QScriptValue> >(&engine);

	logineBoardClass = new BoardClass(&engine);
	logineItemClass = new ItemClass(&engine, &objectManager);
}

void ScriptedRulesetPrivate::initItemArray(const QScriptValue &array) {
	Q_Q(ScriptedRuleset);
	QScriptValueIterator it(array);
	while(it.hasNext()) {
		it.next();
		QScriptValue value = it.value();
		QScriptClass *cls = value.scriptClass();
		if(!cls) {
			if(value.isArray()) {
				initItemArray(value);
			}
		} else if(cls == logineItemClass) {
			ItemWrapper *item = ItemClass::valueToWrapper(value);
			item->init(q);
		} else if(cls == logineBoardClass) {
			BoardWrapper *board = BoardClass::valueToWrapper(value);
			board->init(q);
		}
	}
}

ScriptedRuleset::ScriptedRuleset() {
	d_ptr = new ScriptedRulesetPrivate(this);
}

ScriptedRuleset::~ScriptedRuleset() {
	delete d_ptr;
}

void ScriptedRuleset::addItemType(const QString& name, ItemConstructor ctor) {
	Q_D(ScriptedRuleset);

	d->objectManager.addItemType(name, ctor);
}

QScriptValue debug(QScriptContext *context, QScriptEngine *engine) {
	qDebug() << "debug" << context->argument(0).toString();
	return QScriptValue();
}

bool ScriptedRuleset::load(const QString &program) {
	Q_D(ScriptedRuleset);

	d->logine = d->engine.newObject();
	d->engine.globalObject().setProperty("logine", d->logine, QScriptValue::ReadOnly);
	d->logine.setProperty("Board", d->logineBoardClass->constructor());
	d->logine.setProperty("Item", d->logineItemClass->constructor());

	d->engine.globalObject().setProperty("debug", d->engine.newFunction(debug, 1));

	d->ruleset = d->engine.newObject();
	d->engine.globalObject().setProperty("ruleset", d->ruleset, QScriptValue::ReadOnly);
	d->items = d->engine.newArray();
	d->ruleset.setProperty("items", d->items, QScriptValue::ReadOnly);

	d->engine.evaluate(program, "program", 1);
	if(d->engine.hasUncaughtException()) {
		qDebug() << d->engine.uncaughtExceptionLineNumber() << d->engine.uncaughtException().toString()
			<< d->engine.uncaughtExceptionBacktrace();
		return false;
	}

	// init ruleset
	d->initItemArray(d->items);

	return false;
}

Item* ScriptedRuleset::getItem(const QString& name) {
	Q_D(ScriptedRuleset);
	QStringList parts = name.split('/');
	QScriptValue value = d->items;
	foreach(const QString& part, parts) {
		value = value.property(name);
	}
	if(!value.isValid()) return 0;
	QScriptClass *cls = value.scriptClass();
	if(cls == d->logineItemClass) {
		ItemWrapper *wrapper = ItemClass::valueToWrapper(value);
		if(!wrapper) return 0;
		return wrapper->item();
	} else if(cls == d->logineBoardClass) {
		BoardWrapper *wrapper = BoardClass::valueToWrapper(value);
		if(!wrapper) return 0;
		return wrapper->board();
	}
	
	return 0;
}

ItemBoard* ScriptedRuleset::getBoard(const QString& name) {
	Q_D(ScriptedRuleset);
	QStringList parts = name.split('/');
	QScriptValue value = d->items;
	foreach(const QString& part, parts) {
		value = value.property(name);
	}
	if(value.scriptClass() == d->logineBoardClass) {
		BoardWrapper *wrapper = BoardClass::valueToWrapper(value);
		if(!wrapper) return 0;
		return wrapper->board();
	}

	return 0;
}
