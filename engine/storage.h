#ifndef _STORAGE_H_
#define _STORAGE_H_

class Storage {
	friend class Ruleset;
public:
	class Instance;
public:
	Storage();
	virtual ~Storage();
public:
	virtual Instance *create() const = 0;
	int storageId() const;
private:
	void setStorageId(int storageId);
private:
	int m_storageId;
};

class Storage::Instance {
public:
	Instance();
	virtual ~Instance();
public:
	virtual Instance *clone() const = 0;
	virtual void clone(const Instance *other) = 0;
};

#endif
