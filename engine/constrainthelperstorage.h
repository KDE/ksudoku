#ifndef _KSUDOKU_CONSTRAINTHELPERSTORAGE_H_
#define _KSUDOKU_CONSTRAINTHELPERSTORAGE_H_

#include "storage.h"

#include <QtGlobal>

class ConstraintHelper;
class Ruleset;
class Problem;

class ConstraintHelperStoragePrivate;
class ConstraintHelperStorage : public Storage {
	Q_DECLARE_PRIVATE(ConstraintHelperStorage);
public:
	class Instance;
	class Entry {
		public:
			Entry(ConstraintHelper *helper);
		public:
			void setup(Ruleset *rules);
			void activate(Problem *problem) const;
			void resolve(Problem* problem) const;
		private:
			ConstraintHelperStorage *m_storage;
			ConstraintHelper *m_helper;
			int m_index;
	};
public:
    ConstraintHelperStorage();
	virtual ~ConstraintHelperStorage();
public:
	ConstraintHelper *firstActiveHelper(Problem *problem) const;
	int activeHelpers(Problem *problem) const;
	void reset(Problem *problem) const;
	virtual Storage::Instance* create() const;
	static const char* name() { return "constrainthelpers"; }
protected:
	ConstraintHelperStoragePrivate *d_ptr;
};

#endif // _KSUDOKU_CONSTRAINTHELPERSTORAGE_H_
