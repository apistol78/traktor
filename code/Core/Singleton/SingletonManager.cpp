#include <algorithm>
#include "Core/Singleton/SingletonManager.h"
#include "Core/Singleton/Singleton.h"

namespace traktor
{

SingletonManager& SingletonManager::getInstance()
{
	static SingletonManager instance;
	return instance;
}

void SingletonManager::add(Singleton* singleton)
{
	m_singletons.push_back(singleton);
}

void SingletonManager::addBefore(Singleton* singleton, Singleton* dependency)
{
	T_ASSERT (dependency);

	std::vector< Singleton* >::iterator i = std::find(m_singletons.begin(), m_singletons.end(), dependency);
	T_ASSERT (i != m_singletons.end());

	m_singletons.insert(i, singleton);
}

void SingletonManager::addAfter(Singleton* singleton, Singleton* dependency)
{
	T_ASSERT (dependency);

	std::vector< Singleton* >::iterator i = std::find(m_singletons.begin(), m_singletons.end(), dependency);
	T_ASSERT (i != m_singletons.end());

	m_singletons.insert(++i, singleton);
}

SingletonManager::~SingletonManager()
{
	for (std::vector< Singleton* >::iterator i = m_singletons.begin(); i != m_singletons.end(); ++i)
		(*i)->destroy();

	m_singletons.resize(0);
}

}
