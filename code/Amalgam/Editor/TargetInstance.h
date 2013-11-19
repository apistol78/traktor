#ifndef traktor_amalgam_TargetInstance_H
#define traktor_amalgam_TargetInstance_H

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Amalgam/Impl/TargetPerformance.h"

namespace traktor
{
	namespace net
	{

class TcpSocket;

	}

	namespace amalgam
	{

class Platform;
class Target;
class TargetConfiguration;
class TargetConnection;

enum TargetState
{
	TsIdle,
	TsBuilding,
	TsDeploying,
	TsLaunching,
	TsMigrating,
	TsPending
};

class TargetInstance : public Object
{
	T_RTTI_CLASS;

public:
	TargetInstance(const std::wstring& name, const Target* target, const TargetConfiguration* targetConfiguration, const std::wstring& platformName, const Platform* platform);

	void destroy();

	const Guid& getId() const;

	const std::wstring& getName() const;

	const Target* getTarget() const;

	const TargetConfiguration* getTargetConfiguration() const;

	const std::wstring& getPlatformName() const;

	const Platform* getPlatform() const;

	void setDeployHostId(int32_t deployHostId);

	int32_t getDeployHostId() const;

	void setState(TargetState state);

	TargetState getState() const;

	void setBuildProgress(int32_t buildProgress);

	int32_t getBuildProgress() const;

	bool update();

	void addConnection(TargetConnection* connection);

	void removeConnection(TargetConnection* connection);

	RefArray< TargetConnection > getConnections() const;

	std::wstring getOutputPath() const;

	std::wstring getDatabaseName() const;

private:
	Guid m_id;
	std::wstring m_name;
	Ref< const Target > m_target;
	Ref< const TargetConfiguration > m_targetConfiguration;
	std::wstring m_platformName;
	Ref< const Platform > m_platform;
	int32_t m_deployHostId;
	TargetState m_state;
	int32_t m_buildProgress;
	mutable Semaphore m_connectionsLock;
	RefArray< TargetConnection > m_connections;
};

	}
}

#endif	// traktor_amalgam_TargetInstance_H
