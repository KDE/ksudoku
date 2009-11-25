#ifndef _STATEARRAY_H_
#define _STATEARRAY_H_

#include <QtCore/QAtomicInt>

void *saAlloc(size_t size);
void *saRealloc(void *ptr, size_t size);
void saFree(void *data);
int saAllocMore(int alloc, int extra);

class StateArray {
public:
	inline StateArray();
	StateArray(int size, int states, int defaultState = 0);
	inline StateArray(const StateArray& other);
	inline ~StateArray();
	StateArray& operator=(const StateArray& other);
public:
	inline int at(int i) const;
	inline int state(int i) const;
	inline int operator[](int i) const;
	inline int operator[](uint i) const;
	void setState(int i, int state);
	inline int size() const;
	inline int count() const;
	inline int states() const;
	inline int count(int state) const;
	inline int first(int state) const;
	inline int last(int state) const;
	inline int next(int i) const;
	inline int prev(int i) const;
// 	void debugAll() const;
	void resize(int size, int states, int defaultState);
	void realloc(int size, int states, int defaultState);
	inline void resize(int size, int defaultState = 0);
	inline void detach();
	inline bool isDetached() const;
	void reset(int defaultState = 0);
private:
	struct Entry {
		Entry* prev;
		Entry* next;
		union { int state; int size; };
	};
	struct Data {
		QAtomicInt ref;
		int size;
		int states;
		int alloc;
		Entry* data;
		Entry* stateData;
		Entry array[1];
	};
	static Data shared_null;
private:
	void init(Data *data, int defaultState);
	void copy(Data *dest, Data *src, int defaultState);
// 	void debug(Data*data, QChar prefix = ' ') const;
private:
	Data* d;
};

inline StateArray::StateArray() : d(&shared_null) { d->ref.ref(); }
inline StateArray::StateArray(const StateArray& other) : d(other.d) { d->ref.ref(); }
inline StateArray::~StateArray() { if(!d->ref.deref()) saFree(d); }
inline int StateArray::at(int i) const
{ Q_ASSERT(i >= 0 && i < size()); return d->data[i].state; }
inline int StateArray::state(int i) const
{ Q_ASSERT(i >= 0 && i < size()); return d->data[i].state; }
inline int StateArray::operator[](int i) const
{ Q_ASSERT(i >= 0 && i < size()); return d->data[i].state; }
inline int StateArray::operator[](uint i) const
{ Q_ASSERT(i < uint(size())); return d->data[i].state; }
inline int StateArray::size() const { return d->size; }
inline int StateArray::count() const { return d->size; }
inline int StateArray::states() const { return d->states; }
inline int StateArray::count(int state) const
{ Q_ASSERT(state >= 0 && state < states()); return d->stateData[-state].size; }
inline int StateArray::first(int state) const {
	Q_ASSERT(state >= 0 && state < states());
	int res = d->stateData[-state].next - d->data;
	return (res < size()) ? res : -1;
}
inline int StateArray::last(int state) const {
	Q_ASSERT(state >= 0 && state < states());
	int res = d->stateData[-state].prev - d->data;
	return (res < size()) ? res : -1;
}
inline int StateArray::prev(int i) const {
	Q_ASSERT(i >= 0 && i < size());
	int res = d->data[i].prev - d->data;
	return (res < size()) ? res : -1;
}
inline int StateArray::next(int i) const {
	Q_ASSERT(i >= 0 && i < size());
	int res = d->data[i].next - d->data;
	return (res < size()) ? res : -1;
}
inline void StateArray::resize(int size, int defaultState) { resize(size, states(), defaultState); }
inline void StateArray::detach()
{ if (d->ref != 1) realloc(d->size, d->states, 0); }
inline bool StateArray::isDetached() const
{ return d->ref == 1; }

#endif
