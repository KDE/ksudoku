#include "itemwrapper.h"

#include "item.h"

#include <QScriptValue>
#include <QScriptString>

#include <QMetaProperty>


#include <QScriptEngine>

#include "itemclass.h"

Q_DECLARE_METATYPE(Item*);
Q_DECLARE_METATYPE(QVector<Item*>);
Q_DECLARE_METATYPE(QVector<QScriptValue>);

ItemWrapper::ItemWrapper(Item* item, const QString &name, QScriptEngine *engine) {
	m_item = item;
	m_name = name;

	m_object = engine->newQObject(m_item);

	const QMetaObject *meta = item->metaObject();
	for(int i = meta->propertyCount() - 1; i >= 0; --i) {
		QMetaProperty property = meta->property(i);
		PropertyEntry entry;
		int type = property.userType();
		if(type == qMetaTypeId<Item*>()) entry.type = ItemType;
		else if(type == qMetaTypeId<QVector<Item*> >()) entry.type = ItemVectorType;
		else entry.type = QtType;
		entry.propertyIndex = i;
		m_properties.insert(property.name(), entry);
	}
}

ItemWrapper::~ItemWrapper() {
	if(!m_item->parent()) {
		delete m_item;
	}
}

void ItemWrapper::init(Ruleset* rules) {
	m_item->init(rules);
}

bool ItemWrapper::isInitialized() const {
	return m_item->isInitialized();
}

bool ItemWrapper::hasProperty(const QScriptString& name) const {
	return m_properties.find(name.toString()) != m_properties.end();
}

QScriptValue::PropertyFlags ItemWrapper::propertyFlags(const QScriptString& name) const {
	QHash<QString,PropertyEntry>::const_iterator it = m_properties.find(name.toString());

	if(it == m_properties.end()) return 0;

	QScriptValue::PropertyFlags flags = QScriptValue::Undeletable;
	if(!m_item->metaObject()->property(it.value().propertyIndex).isWritable())
		flags |= QScriptValue::ReadOnly;

	return flags;
}

QScriptValue ItemWrapper::property(const QScriptString& name) const {
	QHash<QString,PropertyEntry>::const_iterator it = m_properties.find(name.toString());

	if(it == m_properties.end()) return QScriptValue();

	const PropertyEntry &property = it.value();
	switch(property.type) {
		case QtType: return m_object.property(name);
		default: return property.value;
	}
}

bool ItemWrapper::setProperty(const QScriptString& name, const QScriptValue& value) {
	QHash<QString,PropertyEntry>::iterator it = m_properties.find(name.toString());

	if(it == m_properties.end()) return false;

	PropertyEntry &property = it.value();
	switch(property.type) {
		case QtType:
			m_object.setProperty(name, value);
			return true;
		case ItemType: {
			property.value = value;
			Item *item = ItemClass::valueToWrapper(value)->item();
			m_item->metaObject()->property(property.propertyIndex).write(m_item, QVariant::fromValue(item));
			return true;
		}
		case ItemVectorType: {
			property.value = value;
 			QVector<QScriptValue> valueList = qscriptvalue_cast<QVector<QScriptValue> >(value);
			QVector<Item*> itemList;
			foreach(QScriptValue v, valueList) {
				ItemWrapper *wrapper = ItemClass::valueToWrapper(v);
				if(wrapper && wrapper->item()) itemList << wrapper->item();
			}
			m_item->metaObject()->property(property.propertyIndex).write(m_item, QVariant::fromValue(itemList));
			return true;
		}
		default:
			Q_ASSERT(!"implemented");
	}

	return false;
}
