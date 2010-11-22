#ifndef traktor_amalgam_DeployTargetAction_H
#define traktor_amalgam_DeployTargetAction_H

#include "Core/Ref.h"
#include "Amalgam/Editor/ITargetAction.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstance;

class DeployTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	DeployTargetAction(TargetInstance* targetInstance);

	virtual bool execute();

private:
	Ref< TargetInstance > m_targetInstance;
};

	}
}

#endif	// traktor_amalgam_DeployTargetAction_H