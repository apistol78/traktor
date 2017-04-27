/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Functor/Functor.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadPool.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Render/Shader/External.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderDependencyTracker.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

void scanDependencies(Ref< ShaderDependencyTracker > tracker, db::Database* database, bool& scanThreadActive)
{
	RefArray< db::Instance > shaderGraphInstances;
	db::recursiveFindChildInstances(database->getRootGroup(), db::FindInstanceByType(type_of< ShaderGraph >()), shaderGraphInstances);
	for (RefArray< db::Instance >::const_iterator i = shaderGraphInstances.begin(); i != shaderGraphInstances.end(); ++i)
	{
		Ref< ShaderGraph > shaderGraph = (*i)->getObject< ShaderGraph >();
		if (!shaderGraph)
			continue;

		RefArray< External > externalNodes;
		shaderGraph->findNodesOf< External >(externalNodes);
		for (RefArray< External >::const_iterator j = externalNodes.begin(); j != externalNodes.end(); ++j)
			tracker->addDependency((*i)->getGuid(), (*j)->getFragmentGuid());
	}
	scanThreadActive = false;
}

void scanDependencies(Ref< ShaderDependencyTracker > tracker, db::Database* database, Guid shader, bool& scanThreadActive)
{
	Ref< db::Instance > instance = database->getInstance(shader);
	if (instance)
	{
		Ref< ShaderGraph > shaderGraph = instance->getObject< ShaderGraph >();
		if (shaderGraph)
		{
			tracker->removeDependencies(shader);

			RefArray< External > externalNodes;
			shaderGraph->findNodesOf< External >(externalNodes);
			for (RefArray< External >::const_iterator j = externalNodes.begin(); j != externalNodes.end(); ++j)
				tracker->addDependency(shader, (*j)->getFragmentGuid());
		}
	}
	scanThreadActive = false;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderDependencyTracker", ShaderDependencyTracker, Object)

ShaderDependencyTracker::ShaderDependencyTracker()
:	m_scanThread(0)
,	m_scanThreadActive(false)
{
}

ShaderDependencyTracker::~ShaderDependencyTracker()
{
	// Destructor should never be reached as long as a reference to tracker
	// belongs to the scan functor.
	T_ASSERT (!m_scanThreadActive);
}

void ShaderDependencyTracker::destroy()
{
	if (m_scanThreadActive)
	{
		ThreadPool::getInstance().join(m_scanThread);
		T_ASSERT(!m_scanThreadActive);
	}
}

void ShaderDependencyTracker::scan(db::Database* database)
{
	if (m_scanThreadActive)
	{
		ThreadPool::getInstance().join(m_scanThread);
		T_ASSERT (!m_scanThreadActive);
	}

	m_scanThreadActive = true;
	ThreadPool::getInstance().spawn(makeStaticFunctor< Ref< ShaderDependencyTracker >, db::Database*, bool& >(&scanDependencies, this, database, m_scanThreadActive), m_scanThread);
}

void ShaderDependencyTracker::scan(db::Database* database, const Guid& shader)
{
	if (m_scanThreadActive)
	{
		ThreadPool::getInstance().join(m_scanThread);
		T_ASSERT (!m_scanThreadActive);
	}

	m_scanThreadActive = true;
	ThreadPool::getInstance().spawn(makeStaticFunctor< Ref< ShaderDependencyTracker >, db::Database*, Guid, bool& >(&scanDependencies, this, database, shader, m_scanThreadActive), m_scanThread);
}

void ShaderDependencyTracker::addListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_listeners.push_back(listener);
	for (std::set< Dependency >::const_iterator i = m_dependencies.begin(); i != m_dependencies.end(); ++i)
		listener->dependencyAdded(i->from, i->to);
}

void ShaderDependencyTracker::removeListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	std::vector< IListener* >::iterator i = std::find(m_listeners.begin(), m_listeners.end(), listener);
	if (i != m_listeners.end())
		m_listeners.erase(i);
}

void ShaderDependencyTracker::addDependency(const Guid& fromShader, const Guid& toShader)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	Dependency dependency = { fromShader, toShader };
	if (m_dependencies.insert(dependency).second)
	{
		for (std::vector< IListener* >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
			(*i)->dependencyAdded(fromShader, toShader);
	}
}

void ShaderDependencyTracker::removeDependency(const Guid& fromShader, const Guid& toShader)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	Dependency dependency = { fromShader, toShader };
	if (m_dependencies.erase(dependency) != 0)
	{
		for (std::vector< IListener* >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
			(*i)->dependencyRemoved(fromShader, toShader);
	}
}

void ShaderDependencyTracker::removeDependencies(const Guid& fromShader)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	
	std::set< Guid > toShaders;
	for (std::set< Dependency >::const_iterator i = m_dependencies.begin(); i != m_dependencies.end(); ++i)
	{
		if (i->from == fromShader)
			toShaders.insert(i->to);
	}

	for (std::set< Guid >::const_iterator i = toShaders.begin(); i != toShaders.end(); ++i)
		removeDependency(fromShader, *i);
}

bool ShaderDependencyTracker::Dependency::operator < (const Dependency& rh) const
{
	if (from < rh.from)
		return true;
	else if (from > rh.from)
		return false;
	else if (to < rh.to)
		return true;
	else if (to > rh.to)
		return false;
	else
		return false;
}

	}
}
