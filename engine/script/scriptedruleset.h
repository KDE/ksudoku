#ifndef SCRIPTEDRULESET_H
#define SCRIPTEDRULESET_H

#include "ruleset.h"

class ItemBoard;

typedef Item * (*ItemConstructor)();

class ScriptedRulesetPrivate;
class ScriptedRuleset : public Ruleset {
	Q_DECLARE_PRIVATE(ScriptedRuleset)
public:
	ScriptedRuleset();
	~ScriptedRuleset();
public:
	template<class Type> inline void addItemType(const QString &name);
	bool load(const QString &program);
	Item *getItem(const QString &name);
	ItemBoard *getBoard(const QString &name);
private:
	void addItemType(const QString &name, ItemConstructor);
protected:
	ScriptedRulesetPrivate *d_ptr;
};

template <class T>
Item *itemConstructHelper() {
	return new T();
}

template<class T>
inline void ScriptedRuleset::addItemType(const QString &name) {
	addItemType(name, itemConstructHelper<T>);
}

#endif // SCRIPTEDRULESET_H
