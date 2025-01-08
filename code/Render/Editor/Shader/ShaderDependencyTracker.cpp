/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/ShaderDependencyTracker.h"

#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadPool.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderDependencyTracker", ShaderDependencyTracker, Object)

ShaderDependencyTracker::ShaderDependencyTracker()
	: m_scanThread(nullptr)
{
}

ShaderDependencyTracker::~ShaderDependencyTracker()
{
	// Destructor should never be reached as long as a reference to tracker
	// belongs to the scan functor.
	T_ASSERT(m_scanThread == nullptr);
}

void ShaderDependencyTracker::destroy()
{
	if (m_scanThread)
	{
		ThreadPool::getInstance().join(m_scanThread);
		m_scanThread = nullptr;
	}
}

void ShaderDependencyTracker::scan(db::Database* database)
{
	if (m_scanThread)
	{
		ThreadPool::getInstance().join(m_scanThread);
		m_scanThread = nullptr;
	}

	ThreadPool::getInstance().spawn(
		[=, this]() {
		RefArray< db::Instance > shaderGraphInstances;
		db::recursiveFindChildInstances(database->getRootGroup(), db::FindInstanceByType(type_of< ShaderGraph >()), shaderGraphInstances);
		for (auto shaderGraphInstance : shaderGraphInstances)
		{
			Ref< ShaderGraph > shaderGraph = shaderGraphInstance->getObject< ShaderGraph >();
			if (shaderGraph)
				for (auto externalNode : shaderGraph->findNodesOf< External >())
					addDependency(shaderGraphInstance->getGuid(), externalNode->getFragmentGuid());
		}
		},
		m_scanThread);
}

void ShaderDependencyTracker::scan(db::Database* database, const Guid& shader)
{
	if (m_scanThread)
	{
		ThreadPool::getInstance().join(m_scanThread);
		m_scanThread = nullptr;
	}

	ThreadPool::getInstance().spawn(
		[=, this]() {
		Ref< db::Instance > instance = database->getInstance(shader);
		if (instance)
		{
			Ref< ShaderGraph > shaderGraph = instance->getObject< ShaderGraph >();
			if (shaderGraph)
			{
				removeDependencies(shader);
				for (auto externalNode : shaderGraph->findNodesOf< External >())
					addDependency(shader, externalNode->getFragmentGuid());
			}
		}
		},
		m_scanThread);
}

void ShaderDependencyTracker::addListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_listeners.push_back(listener);
	for (const auto& dependency : m_dependencies)
		listener->dependencyAdded(dependency.from, dependency.to);
}

void ShaderDependencyTracker::removeListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
	if (it != m_listeners.end())
		m_listeners.erase(it);
}

void ShaderDependencyTracker::addDependency(const Guid& fromShader, const Guid& toShader)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	Dependency dependency = { fromShader, toShader };
	if (m_dependencies.insert(dependency).second)
		for (auto listener : m_listeners)
			listener->dependencyAdded(fromShader, toShader);
}

void ShaderDependencyTracker::removeDependency(const Guid& fromShader, const Guid& toShader)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	const Dependency dependency = { fromShader, toShader };
	if (m_dependencies.erase(dependency) != 0)
		for (auto listener : m_listeners)
			listener->dependencyRemoved(fromShader, toShader);
}

void ShaderDependencyTracker::removeDependencies(const Guid& fromShader)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::set< Guid > toShaders;
	for (const auto& dependency : m_dependencies)
		if (dependency.from == fromShader)
			toShaders.insert(dependency.to);
	for (const auto& toShader : toShaders)
		removeDependency(fromShader, toShader);
}

bool ShaderDependencyTracker::Dependency::operator<(const Dependency& rh) const
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
