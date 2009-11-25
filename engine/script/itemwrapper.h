#ifndef _KSUDOKU_ITEMWRAPPER_H_
#define _KSUDOKU_ITEMWRAPPER_H_

#include <QMetaType>
#include <QScriptValue>
#include <QHash>

class Item;
class Ruleset;

class ItemWrapper {
private:
	enum PropertyType {
		QtType,
		ItemType,
		ItemVectorType,
		BoardType,
	};
	struct PropertyEntry {
		PropertyType type;
		int propertyIndex;
		QScriptValue value;
	};
public:
	ItemWrapper(Item *item, const QString &name, QScriptEngine *engine);
	~ItemWrapper();
public:
	inline Item *item() const { return m_item; }
	inline QString name() const { return m_name; }
	bool isInitialized() const;
	bool hasProperty(const QScriptString& name) const;
	QScriptValue::PropertyFlags propertyFlags(const QScriptString &name) const;
	QScriptValue property(const QScriptString &name) const;
	bool setProperty(const QScriptString &name, const QScriptValue &value);
public:
	void init(Ruleset *rules);
private:
	QHash<QString,PropertyEntry> m_properties;
	QScriptValue m_object;
	Item *m_item;
	QString m_name;
};

Q_DECLARE_METATYPE(ItemWrapper*);

#endif // _KSUDOKU_ITEMWRAPPER_H_
