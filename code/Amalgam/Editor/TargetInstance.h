#ifndef traktor_amalgam_TargetInstance_H
#define traktor_amalgam_TargetInstance_H

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Amalgam/Impl/TargetPerformance.h"

namespace traktor
{
	namespace net
	{

class TcpSocket;

	}

	namespace amalgam
	{

class Target;
class TargetConnection;

enum TargetState
{
	TsIdle,
	TsPending,
	TsBuilding,
	TsDeploying,
	TsLaunching
};

class TargetInstance : public Object
{
	T_RTTI_CLASS;

public:
	TargetInstance(const std::wstring& name, const Target* target);

	void destroy();

	const Guid& getId() const;

	const std::wstring& getName() const;

	const Target* getTarget() const;

	void setDeployHostId(int32_t deployHostId);

	int32_t getDeployHostId() const;

	void setState(TargetState state);

	TargetState getState() const;

	void setBuildProgress(int32_t buildProgress);

	int32_t getBuildProgress() const;

	void update();

	void addConnection(TargetConnection* connection);

	const RefArray< TargetConnection >& getConnections() const;

private:
	Guid m_id;
	std::wstring m_name;
	Ref< const Target > m_target;
	int32_t m_deployHostId;
	TargetState m_state;
	int32_t m_buildProgress;
	RefArray< TargetConnection > m_connections;
};

	}
}

#endif	// traktor_amalgam_TargetInstance_H
