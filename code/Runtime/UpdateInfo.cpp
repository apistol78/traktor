#include "Runtime/UpdateInfo.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.UpdateInfo", UpdateInfo, Object)

UpdateInfo::UpdateInfo()
:	m_totalTime(0.0f)
,	m_stateTime(0.0f)
,	m_simulationTime(0.0f)
,	m_simulationDeltaTime(0.0f)
,	m_simulationFrequency(0)
,	m_frameDeltaTime(0.0f)
,	m_frame(0)
,	m_runningSlow(false)
{
}

	}
}
