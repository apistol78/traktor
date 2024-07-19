/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/TargetConnection.h"
#include "Runtime/Editor/TargetInstance.h"
#include "Runtime/Editor/Deploy/Target.h"
#include "Runtime/Editor/Deploy/TargetConfiguration.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"

namespace traktor::runtime
{
	namespace
	{

std::wstring mangleName(std::wstring name)
{
	name = replaceAll(name, ' ', '_');
	name = replaceAll(name, ',', '_');
	name = replaceAll(name, ';', '_');
	name = replaceAll(name, '|', '_');
	return name;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetInstance", TargetInstance, Object)

TargetInstance::TargetInstance(const std::wstring& name, const Target* target, const TargetConfiguration* targetConfiguration, const std::wstring& platformName, const Platform* platform)
:	m_id(Guid::create())
,	m_name(name)
,	m_target(target)
,	m_targetConfiguration(targetConfiguration)
,	m_platformName(platformName)
,	m_platform(platform)
,	m_deployHostId(-1)
,	m_state(TsIdle)
{
}

void TargetInstance::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_connectionsLock);
	for (auto connection : m_connections)
		connection->destroy();
	m_connections.clear();
}

const Guid& TargetInstance::getId() const
{
	return m_id;
}

const std::wstring& TargetInstance::getName() const
{
	return m_name;
}

const Target* TargetInstance::getTarget() const
{
	return m_target;
}

const TargetConfiguration* TargetInstance::getTargetConfiguration() const
{
	return m_targetConfiguration;
}

const std::wstring& TargetInstance::getPlatformName() const
{
	return m_platformName;
}

const Platform* TargetInstance::getPlatform() const
{
	return m_platform;
}

void TargetInstance::setDeployHostId(int32_t deployHostId)
{
	m_deployHostId = deployHostId;
}

int32_t TargetInstance::getDeployHostId() const
{
	return m_deployHostId;
}

void TargetInstance::setState(TargetState state)
{
	m_state = state;
}

TargetState TargetInstance::getState() const
{
	return m_state;
}

void TargetInstance::setBuildProgress(int32_t buildProgress)
{
	m_buildProgress = buildProgress;
}

int32_t TargetInstance::getBuildProgress() const
{
	return m_buildProgress;
}

bool TargetInstance::update()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_connectionsLock);
	for (auto connection : m_connections)
	{
		if (!connection->update())
		{
			log::info << L"Target disconnected; connection removed." << Endl;
			m_connections.remove(connection);
			return true;
		}
	}
	return !m_connections.empty();
}

void TargetInstance::addConnection(TargetConnection* connection)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_connectionsLock);
	m_connections.push_back(connection);
}

void TargetInstance::removeConnection(TargetConnection* connection)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_connectionsLock);
	m_connections.remove(connection);
}

RefArray< TargetConnection > TargetInstance::getConnections() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_connectionsLock);
	return m_connections;
}

std::wstring TargetInstance::getOutputPath() const
{
	return L"output/" + mangleName(m_name) + L"/" + mangleName(m_targetConfiguration->getName());
}

std::wstring TargetInstance::getDatabaseName() const
{
	return mangleName(m_name) + L"|" + mangleName(m_targetConfiguration->getName());
}

}
