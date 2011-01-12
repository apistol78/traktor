#ifndef traktor_amalgam_LaunchTargetAction_H
#define traktor_amalgam_LaunchTargetAction_H

#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Amalgam/Editor/ITargetAction.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstance;

class LaunchTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	LaunchTargetAction(TargetInstance* targetInstance, const Guid& activeGuid);

	virtual bool execute();

private:
	Ref< TargetInstance > m_targetInstance;
	Guid m_activeGuid;
};

	}
}

#endif	// traktor_amalgam_LaunchTargetAction_H