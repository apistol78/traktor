#include <cstdlib>
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

TheaterController::TheaterController(const RefArray< const Act >& acts, bool repeatActs)
:	m_acts(acts)
,	m_repeatActs(repeatActs)
{
	for (int32_t i = 0; i < (int32_t)m_acts.size(); ++i)
	{
		m_totalDuration += m_acts[i]->getDuration();
		m_hasInfinite |= m_acts[i]->isInfinite();
	}
}

void TheaterController::update(scene::Scene* scene, float time, float deltaTime)
{
	if (m_acts.empty() || traktor::abs(time - m_lastTime) <= FUZZY_EPSILON)
		return;

	// Do nothing if we're finished.
	if (!m_repeatActs && !m_hasInfinite && time > m_totalDuration)
		return;

	// Repeat all acts if no infinite act exist.
	if (m_repeatActs && !m_hasInfinite)
		time = std::fmod(time, m_totalDuration);

	// Figure out which act we're in; as time might not be continous we need to find this each update.
	const Act* act = nullptr;
	float actStartTime = 0.0f;
	for (int32_t i = 0; i < (int32_t)m_acts.size(); ++i)
	{
		float actDuration = m_acts[i]->getDuration();
		if (time >= actStartTime && (m_acts[i]->isInfinite() || time <= actStartTime + actDuration - FUZZY_EPSILON))
		{
			act = m_acts[i];
			break;
		}
		actStartTime += actDuration;
	}

	if (act)
		act->update(scene, time - actStartTime, deltaTime);

	m_lastTime = time;
}

	}
}
