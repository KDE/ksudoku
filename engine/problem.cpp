#include "problem.h"

#include "ruleset.h"

#include <iostream>

#include "choiceitem.h"


Problem::Problem() {
	m_rules = 0;
}

Problem::Problem(const Ruleset *rules) {
	m_rules = rules;
	if(rules) {
		QVector<Storage*>::iterator it;
		QVector<Storage*> storages(rules->storages());
		for(it = storages.begin(); it != storages.end(); ++it) {
			m_storages << (*it)->create();
		}
	}
}

Problem::Problem(const Problem &problem) {
	QVector<Storage::Instance*>::const_iterator it;
	for(it = problem.m_storages.begin(); it != problem.m_storages.end(); ++it) {
		m_storages << (*it)->clone();
	}
	
	m_rules = problem.m_rules;
}

Problem::~Problem() {
	QVector<Storage::Instance*>::const_iterator it;
	for(it = m_storages.begin(); it != m_storages.end(); ++it) {
		delete *it;
	}
}

#include <QtDebug>
Problem &Problem::operator=(const Problem &other) {
	if(&other == this) return *this;
	
	if(m_rules) {
		if(other.m_rules) {
			Q_ASSERT(m_rules == other.m_rules);
			QVector<Storage::Instance*>::const_iterator src, dest;
			for(src = other.m_storages.constBegin(), dest = m_storages.constBegin();
				src != other.m_storages.constEnd(); ++src, ++dest)
			{
				(*dest)->clone(*src);
			}
		} else {
			QVector<Storage::Instance*>::const_iterator it;
			for(it = m_storages.constBegin(); it != m_storages.constEnd(); ++it) {
				delete *it;
			}
			m_storages.resize(0);
			m_rules = 0;
		}
	} else {
		if(other.m_rules) {
			QVector<Storage::Instance*>::const_iterator src;
			for(src = other.m_storages.constBegin(); src != other.m_storages.constEnd(); ++src) {
				m_storages << (*src)->clone();
			}
			m_rules = other.m_rules;
		}
	}
	return *this;
}
