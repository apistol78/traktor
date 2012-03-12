#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetConfiguration.h"
#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace amalgam
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetInstance", TargetInstance, Object)

TargetInstance::TargetInstance(const std::wstring& name, const Target* target, const TargetConfiguration* targetConfiguration, const Platform* platform)
:	m_id(Guid::create())
,	m_name(name)
,	m_target(target)
,	m_targetConfiguration(targetConfiguration)
,	m_platform(platform)
,	m_deployHostId(0)
,	m_state(TsIdle)
{
}

void TargetInstance::destroy()
{
	for (RefArray< TargetConnection >::iterator i = m_connections.begin(); i != m_connections.end(); ++i)
		(*i)->destroy();

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

void TargetInstance::update()
{
	for (RefArray< TargetConnection >::iterator i = m_connections.begin(); i != m_connections.end(); )
	{
		if (!(*i)->update())
		{
			i = m_connections.erase(i);
			log::info << L"Target disconnected; connection removed" << Endl;
		}
		else
			++i;
	}
}

void TargetInstance::addConnection(TargetConnection* connection)
{
	m_connections.push_back(connection);
}

void TargetInstance::removeConnection(TargetConnection* connection)
{
	m_connections.remove(connection);
}

const RefArray< TargetConnection>& TargetInstance::getConnections() const
{
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
}
