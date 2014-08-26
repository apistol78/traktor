#include "Animation/Animation/StateContext.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StateContext", StateContext, Object)

StateContext::StateContext()
:	m_time(0.0f)
,	m_duration(0.0f)
,	m_indexHint(-1)
{
}

	}
}
