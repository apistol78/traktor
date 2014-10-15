#include <limits>
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Theater/Act.h"
#include "Theater/TheaterController.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterController", TheaterController, scene::ISceneController)

TheaterController::TheaterController(const RefArray< const Act >& acts)
:	m_acts(acts)
,	m_lastTime(-1.0f)
{
}

void TheaterController::update(scene::Scene* scene, float time, float deltaTime)
{
	if (m_acts.empty() || traktor::abs(time - m_lastTime) <= FUZZY_EPSILON)
		return;

	float actStartTime = 0.0f;
	for (int32_t i = 0; i < int32_t(m_acts.size()); ++i)
	{
		float actDuration = m_acts[i]->getDuration();
		if (time >= actStartTime && (m_acts[i]->isInfinite() || time <= actStartTime + actDuration + FUZZY_EPSILON))
		{
			m_acts[i]->update(scene, time - actStartTime, deltaTime);
			break;
		}
		actStartTime += actDuration;
	}

	m_lastTime = time;
}

float TheaterController::getActStartTime(int32_t act) const
{
	float actStartTime = 0.0f;
	for (int32_t i = 0; i < int32_t(m_acts.size()); ++i)
	{
		if (act == i || m_acts[i]->isInfinite())
			return actStartTime;
		actStartTime += m_acts[i]->getDuration();
	}
	return actStartTime;
}

	}
}
