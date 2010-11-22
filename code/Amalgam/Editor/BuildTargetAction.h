#ifndef traktor_amalgam_BuildTargetAction_H
#define traktor_amalgam_BuildTargetAction_H

#include "Core/Ref.h"
#include "Amalgam/Editor/ITargetAction.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstance;

class BuildTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	BuildTargetAction(TargetInstance* targetInstance);

	virtual bool execute();

private:
	Ref< TargetInstance > m_targetInstance;
};

	}
}

#endif	// traktor_amalgam_BuildTargetAction_H