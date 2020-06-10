#include <algorithm>
#include "Core/Singleton/ISingleton.h"
#include "Core/Singleton/SingletonManager.h"

namespace traktor
{

SingletonManager& SingletonManager::getInstance()
{
	static SingletonManager instance;
	return instance;
}

void SingletonManager::add(ISingleton* singleton)
{
	m_singletons.push_back(singleton);
}

void SingletonManager::addBefore(ISingleton* singleton, ISingleton* dependency)
{
	T_ASSERT(dependency);

	auto it = std::find(m_singletons.begin(), m_singletons.end(), dependency);
	T_ASSERT(it != m_singletons.end());

	m_singletons.insert(it, singleton);
}

void SingletonManager::addAfter(ISingleton* singleton, ISingleton* dependency)
{
	T_ASSERT(dependency);

	auto it = std::find(m_singletons.begin(), m_singletons.end(), dependency);
	T_ASSERT(it != m_singletons.end());

	m_singletons.insert(++it, singleton);
}

void SingletonManager::destroy()
{
	for (auto singleton : m_singletons)
		singleton->destroy();
	m_singletons.clear();
}

SingletonManager::~SingletonManager()
{
	destroy();
}

}
