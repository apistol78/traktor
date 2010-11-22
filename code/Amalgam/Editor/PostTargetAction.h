#ifndef traktor_amalgam_PostTargetAction_H
#define traktor_amalgam_PostTargetAction_H

#include "Amalgam/Editor/ITargetAction.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace amalgam
	{

class PostTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	PostTargetAction(TargetInstance* targetInstance, TargetState targetState);

	virtual bool execute();

private:
	Ref< TargetInstance > m_targetInstance;
	TargetState m_targetState;
};

	}
}

#endif	// traktor_amalgam_PostTargetAction_H
