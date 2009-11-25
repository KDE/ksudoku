#ifndef _MARKERSTORAGE_H_
#define _MARKERSTORAGE_H_

#include <QtCore/QtGlobal>
#include "storage.h"

class Ruleset;
class Problem;

class MarkerStoragePrivate;
class MarkerStorage : public Storage {
public:
	class Instance;
	class Entry {
		public:
			Entry(int size = 0);
		public:
			int size() const;
			void setSize(int size);
			void setup(Ruleset *rules);

			bool marker(const Problem *problem, int index) const;
			void setMarker(Problem *problem, int index, bool state) const;
		private:
			MarkerStorage *m_storage;
			int m_index;
			int m_size;
	};
public:
	MarkerStorage();
	virtual ~MarkerStorage();
public:
	void reset(Problem *problem) const;
	Storage::Instance *create() const;
	static const char* name() { return "markers"; }
protected:
	MarkerStoragePrivate *d_ptr;
private:
	Q_DECLARE_PRIVATE(MarkerStorage);
};

#endif
