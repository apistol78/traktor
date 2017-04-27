/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	T_ASSERT (dependency);

	std::vector< ISingleton* >::iterator i = std::find(m_singletons.begin(), m_singletons.end(), dependency);
	T_ASSERT (i != m_singletons.end());

	m_singletons.insert(i, singleton);
}

void SingletonManager::addAfter(ISingleton* singleton, ISingleton* dependency)
{
	T_ASSERT (dependency);

	std::vector< ISingleton* >::iterator i = std::find(m_singletons.begin(), m_singletons.end(), dependency);
	T_ASSERT (i != m_singletons.end());

	m_singletons.insert(++i, singleton);
}

void SingletonManager::destroy()
{
	for (std::vector< ISingleton* >::iterator i = m_singletons.begin(); i != m_singletons.end(); ++i)
		(*i)->destroy();

	m_singletons.resize(0);
}

SingletonManager::~SingletonManager()
{
	destroy();
}

}
