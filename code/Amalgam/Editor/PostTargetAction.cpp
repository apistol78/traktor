#include "Amalgam/Editor/PostTargetAction.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.PostTargetAction", PostTargetAction, ITargetAction)

PostTargetAction::PostTargetAction(TargetInstance* targetInstance, TargetState targetState)
:	m_targetInstance(targetInstance)
,	m_targetState(targetState)
{
}

bool PostTargetAction::execute()
{
	m_targetInstance->setState(m_targetState);
	return true;
}

	}
}
