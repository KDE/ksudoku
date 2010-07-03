#ifndef _KSUDOKU_ELIMINATIONSTORAGE_H_
#define _KSUDOKU_ELIMINATIONSTORAGE_H_

#include "storage.h"

#include <QtGlobal>

class Ruleset;
class Problem;

class IVariableItem;
class EliminationStoragePrivate;
class EliminationStorage : public Storage {
public:
	class Instance;
	class Entry {
		public:
			explicit Entry(IVariableItem *item = 0, int possibilities = 0);
		public:
			int possibilities() const;
			void setPossibilities(int possibilities);
			IVariableItem *item() const;
			void setItem(IVariableItem *item);
			void setup(Ruleset *rules);

			int possibilitiesLeft(const Problem *problem) const;
			void incPossibilitiesLeft(Problem *problem) const;
			void decPossibilitiesLeft(Problem *problem) const;
			void setPossibilitiesLeft(Problem *problem, int count) const;
		public:
		private:
			EliminationStorage *m_storage;
			IVariableItem *m_item;
			int m_index;
			int m_possibilities;
	};
public:
	EliminationStorage();
	virtual ~EliminationStorage();
public:
	IVariableItem *itemWithLeastPossibilitiesLeft(const Problem *problem) const;
	bool isFinished(const Problem *problem) const;
	void reset(Problem* problem) const;
	Storage::Instance *create() const;
	static const char* name() { return "eliminations"; }
protected:
	EliminationStoragePrivate *d_ptr;
private:
	Q_DECLARE_PRIVATE(EliminationStorage)
};

#endif
