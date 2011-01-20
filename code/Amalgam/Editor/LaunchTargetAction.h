#ifndef traktor_amalgam_LaunchTargetAction_H
#define traktor_amalgam_LaunchTargetAction_H

#include "Amalgam/Editor/ITargetAction.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace amalgam
	{

class PlatformInstance;
class TargetInstance;

class LaunchTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	LaunchTargetAction(PlatformInstance* platformInstance, TargetInstance* targetInstance);

	virtual bool execute();

private:
	Ref< PlatformInstance > m_platformInstance;
	Ref< TargetInstance > m_targetInstance;
};

	}
}

#endif	// traktor_amalgam_LaunchTargetAction_H