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
class TargetManager;

class LaunchTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	LaunchTargetAction(TargetInstance* targetInstance, TargetManager* targetManager, const Guid& activeGuid);

	virtual bool execute();

private:
	Ref< TargetInstance > m_targetInstance;
	Ref< TargetManager > m_targetManager;
	Guid m_activeGuid;
};

	}
}

#endif	// traktor_amalgam_LaunchTargetAction_H