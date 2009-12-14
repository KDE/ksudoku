#include "./itemprototype.h"

#include <QScriptValue>
#include <QVariant>

#include "item.h"
#include "itemwrapper.h"

ItemPrototype::ItemPrototype(QObject *parent)
	: QObject(parent)
{
}

ItemWrapper* ItemPrototype::thisItem() const {
	return thisObject().data().toVariant().value<ItemWrapper*>();
}

QString ItemPrototype::toString() const {
	ItemWrapper *wrapper = thisItem();
	return QString("[object logine.Item(%1)]").arg(wrapper->name());
}

#include "itemprototype.moc"