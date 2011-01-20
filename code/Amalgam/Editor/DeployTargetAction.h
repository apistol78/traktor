#ifndef traktor_amalgam_DeployTargetAction_H
#define traktor_amalgam_DeployTargetAction_H

#include "Amalgam/Editor/ITargetAction.h"
#include "Core/Guid.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace amalgam
	{

class PlatformInstance;
class TargetInstance;

class DeployTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	DeployTargetAction(PlatformInstance* platformInstance, TargetInstance* targetInstance, const Guid& activeGuid);

	virtual bool execute();

private:
	Ref< PlatformInstance > m_platformInstance;
	Ref< TargetInstance > m_targetInstance;
	Guid m_activeGuid;
};

	}
}

#endif	// traktor_amalgam_DeployTargetAction_H