// TODO statearray might need to be redone for licensing reasons
#include "statearray.h"

#include <cstdlib>
#include <QtCore/qglobal.h>
#include <QtCore/qatomic.h>

// this functions were taken from Qts source
// they were inserted here as it is not part of Qts public API
// TODO replace them for licensing reasons
void* saAlloc(size_t alloc) { return ::malloc(alloc); }
void* saRealloc(void *ptr, size_t size) { return ::realloc(ptr, size); }
void saFree(void* ptr) { ::free(ptr); }
int saAllocMore(int alloc, int extra) {
    const int page = 1 << 12;
    int nalloc;
    alloc += extra;
    if (alloc < 1<<6) {
        nalloc = (1<<3) + ((alloc >>3) << 3);
    } else  {
        nalloc = (alloc < page) ? 1 << 3 : page;
        while (nalloc < alloc) {
			// TODO replace this assert with something equal to the code below
			Q_ASSERT(nalloc > 0);
            nalloc *= 2;
        }
    }
    return nalloc - extra;
}


/*!
	\class StateArray
	\brief The StateArray class provides an array of states.
	
	\reentrant
	
	StateArray is a bidirectional association of indices and states. Each index has exactly one state and arbitrary indices can be set to one state. All basic operations can be done in constant time.
*/

/*! \fn StateArray::StateArray()

	Constructs a empty state array.
*/

/*! \fn StateArray::StateArray(const StateArray &other)

	Constructs a copy of \a other.

	This operation takes constant time, because StateArray is
	implicitly shared. This makes returning a StateArray from a
	function very fast. If a shared instance is modified, it will be
	copied (copy-on-write), and that takes linear time.

	\sa operator=()
*/

/*! \fn StateArray::~StateArray()

	Destroys the state array.
*/

/*! \fn int StateArray::at(int i) const

	Returns the state associated with the index \a i.
	
	\sa state(), operator[]()
*/

/*! \fn int StateArray::state(int i) const

	Same as at(\a i)
*/

/*! \fn int StateArray::operator[](int i) const

	Same as at(\a i)
*/

/*! \fn int StateArray::operator[](uint i) const

	\overload
*/

/*! \fn int StateArray::size() const

	Returns the number of entries in this state array.

	\sa isEmpty(), resize(), states()
*/

/*! \fn int StateArray::count(int state) const

	Returns the number of entries which are in state \a state.

	This operation takes constant time.
*/

/*! \fn int StateArray::count() const

	\overload

	Same as size()
*/

/*! \fn int StateArray::states() const

	Returns the maximum number of different states.

	\sa size(), resize()
*/

/*! \fn int StateArray::first(int state) const

	Returns the index of the first entry with the state \a state. If there is no entry with \a state this function returns -1.

	This operation takes constant time.

	Note: The first entry is in this case the first set to
	\a state, not the one with the lowest index.
*/

/*! \fn int StateArray::last(int state) const

	Returns the index of the last entry with the state \a state. If there is no entry with \a state this function returns -1.

	This operation takes constant time.

	Note: The last entry is in this case the last set to
	\a state, not the one with the highest index.
*/

/*! \fn int StateArray::prev(int i) const

	Returns the index of the entry set to the state of \a i right before \a i.

	This operation takes constant time.

	If there was no entry set to the state of \a i before \a i this function returns -1.
*/

/*! \fn int StateArray::next(int i) const

	Returns the index of the entry set to the state of \a i right after \a i.

	This operation takes constant time.

	If there was no entry set to the state of \a i after \a i this function returns -1.
*/

/*! \fn void StateArray::resize(int size)
	
	\overload

	Same as resize(\a size, states())
*/


StateArray::Data StateArray::shared_null = { (1), 0, 0, 0, shared_null.array, shared_null.array, {{0,0,{0}}} };

/*!
	Constructs a state array with \a size entries set to state \a defaultState and at maximum \a states different states.

	If \a states is smaller than 1, a null state array is constructed.
*/

StateArray::StateArray(int size, int states, int defaultState) {
	if(states < 1 || size <= 0) {
		d = &shared_null;
	} else {
		int alloc = size + states;
		d = static_cast<Data*>(saAlloc(sizeof(Data) + sizeof(Entry)*(alloc-1)));
		if(!d) {
			d = &shared_null;
		} else {
			d->ref = 0;
			d->size = size;
			d->states = states;
			d->alloc = alloc;
			d->data = d->array;
			d->stateData = &d->array[alloc-1];
			init(d, defaultState);
		}
	}
	d->ref.ref();
}

/*!
    Assigns \a other to this byte array and returns a reference to
    this byte array.
*/
StateArray& StateArray::operator=(const StateArray& other) {
	other.d->ref.ref();
	if(!d->ref.deref())
		saFree(d);
	d = other.d;
	return *this;
}

/*!
	Sets the state of the entry at pos \a i to the state \a state.

	This operation takes constant time.
*/

void StateArray::setState(int i, int state) {
	Q_ASSERT(i >= 0 && i < size() && state >= 0 && state < states());
	detach();
	Entry* entry = d->data + i;
	Entry* stateEntry = d->stateData - state;
	entry->prev->next = entry->next;
	entry->next->prev = entry->prev;
	stateEntry->prev->next = entry;
	entry->prev = stateEntry->prev;
	entry->next = stateEntry;
	stateEntry->prev = entry;
	
	d->stateData[-entry->state].size--;
	entry->state = state;
	stateEntry->size++;
}

void StateArray::resize(int size, int states, int defaultState) {
	if(d->size == size && d->states == states) return;

	if(size < 0 || states <= 0 || defaultState < 0 || defaultState >= states) {
		Data *x = &shared_null;
		x->ref.ref();
		if(!d->ref.deref())
			saFree(d);
		d = x;
	} else if(d == &shared_null) {
		int alloc = size + states;
		Data *x = static_cast<Data*>(saAlloc(sizeof(Data) + sizeof(Entry)*(alloc-1)));
		
		if(!x) return;
		x->ref = 1;
		x->size = size;
		x->states = states;
		x->alloc = alloc;
		x->data = x->array;
		x->stateData = &x->array[alloc-1];
		init(x, defaultState);
		d->ref.deref(); // cannot be 0
		d = x;
	} else {
		realloc(size, states, defaultState);
	}
}

void StateArray::realloc(int size, int states, int defaultState) {
	int alloc = saAllocMore(sizeof(Entry)*(size+states), sizeof(Data)-sizeof(Entry)) / sizeof(Entry);
	// TODO check whether a simple resize is sufficient
	Data *x = static_cast<Data*>(saAlloc(sizeof(Data) + sizeof(Entry)*(alloc-1)));
	if(!x) {
		shared_null.ref.ref();
		if(!d->ref.deref())
			saFree(d);
		d = &shared_null;
		return;
	}
	x->ref = 1;
	x->size = size;
	x->states = states;
	x->alloc = alloc;
	x->data = x->array;
	x->stateData = &x->array[alloc-1];
	copy(x, d, defaultState);
	if(!d->ref.deref())
		saFree(d);
	d = x;
}

// inits the states and entries in data
void StateArray::init(Data *data, int defaultState) {
	// init states
	Entry* state = data->stateData;
	for(int i = data->states; i > 0; --i, --state) {
		state->prev = state;
		state->next = state;
		state->size = 0;
	}
	// insert entries to default State
	state = data->stateData - defaultState;
	Entry* entry = data->data;
	for(int i = data->size; i > 0; --i, ++entry) {
		state->prev->next = entry;
		entry->prev = state->prev;
		entry->next = state;
		state->prev = entry;
		entry->state = defaultState;
	}
	state->size += data->size;
}

// copies the states and entries from src to dest and adapts to the nes size
void StateArray::copy(Data *dest, Data *src, int defaultState) {
	int states = (src->states < dest->states) ? src->states : dest->states;
	Entry* srcState = src->stateData;
	Entry* destState = dest->stateData;

	// copy all entries their equivalent new state
	for(int i = 0; i < states; ++i, --srcState, --destState) {
		Entry* lastDest = destState;
		int j = 0;
		for(Entry* srcEntry = srcState->next; srcEntry != srcState; srcEntry = srcEntry->next) {
			if(srcEntry == srcState) break;
			if(srcEntry - src->data > dest->size) continue;
			Entry* nextDest = srcEntry - src->data + dest->data;
			lastDest->next = nextDest;
			nextDest->prev = lastDest;
			nextDest->state = i;
			++j;
			lastDest = nextDest;
		}
		lastDest->next = destState;
		destState->prev = lastDest;
		destState->size = j;
	}
	// insert new defaultStates
	destState = dest->stateData - states;
	for(int i = states; i < dest->states; ++i, --srcState) {
		destState->prev = destState;
		destState->next = destState;
		destState->size = 0;
	}
	// move all entries with no new state to the defaultState
	destState = dest->stateData - defaultState;
	Entry* lastDest = destState->prev;
	int j = 0;
	for(int i = states; i < src->states; ++i, --srcState) {
		for(Entry* srcEntry = srcState->next; srcEntry != srcState; srcEntry = srcEntry->next) {
			if(srcEntry == srcState) break;
			if(srcEntry - src->data > dest->size) continue;
			Entry* nextDest = srcEntry - src->data + dest->data;
			lastDest->next = nextDest;
			nextDest->prev = lastDest;
			nextDest->state = defaultState;
			++j;
			lastDest = nextDest;
		}
	}
	// move all new entries to the defaultState
	if(src->size < dest->size) {
		lastDest = dest->data + src->size;
		for(int i = src->size; i < dest->size; ++i, ++lastDest) {
			Entry* nextDest = dest->data + i;
			lastDest->next = nextDest;
			nextDest->prev = lastDest;
			nextDest->state = defaultState;
			++j;
			lastDest = nextDest;
		}
		lastDest->next = destState;
		destState->prev = lastDest;
		destState->size += j;
	}
}

// void StateArray::debug(Data*d, QChar prefix) const {
// 	qDebug() << prefix << "Debug Entries";
// 	for(int i = 0; i < size(); ++i) {
// 		qDebug() << prefix << i << ":" << "prev ="<<d->data[i].prev-d->data << "state ="<<d->data[i].state << "next ="<<d->data[i].next-d->data;
// 	}
// 	qDebug() << prefix << "Debug States";
// 	for(int i = 0; i < states(); ++i) {
// 		qDebug() << prefix << i << ":" << "first ="<<d->stateData[-i].next-d->data << "last ="<<d->stateData[-i].prev-d->data << "size ="<<d->stateData[-i].state << "pos ="<<d->stateData-i-d->data;
// 	}
// }
// 
// void StateArray::debugAll() const {
// 	debug(d);
// }

void StateArray::reset(int defaultState) {
	detach();
	init(d, defaultState);
}
