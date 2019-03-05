#include "Runtime/UpdateControl.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.UpdateControl", UpdateControl, Object)

UpdateControl::UpdateControl()
:	m_pause(false)
,	m_timeScale(1.0f)
,	m_simulationFrequency(60.0f)
{
}

	}
}
