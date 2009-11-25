#include "storage.h"

Storage::Instance::Instance() {
}

Storage::Instance::~Instance() {
}

/**
 * \fn Storage *Storage::clone() const
 *
 * Creates a clone of the current storage;
 */

/**
 * \fn void Storage::clone(Storage *other)
 *
 * Changes the internal state to the state of \a other
 */

Storage::Storage() {
	m_storageId = -1;
}

int Storage::storageId() const {
	return m_storageId;
}

void Storage::setStorageId(int storageId) {
	m_storageId = storageId;
}
