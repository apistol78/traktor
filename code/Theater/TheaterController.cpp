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
,	m_current(0)
,	m_lastTime(-1.0f)
,	m_actTime(-1.0f)
{
}

void TheaterController::update(scene::Scene* scene, float time, float deltaTime)
{
	if (m_acts.empty() || traktor::abs(time - m_lastTime) <= FUZZY_EPSILON)
		return;

	if (m_actTime < 0.0f || m_actTime > time)
		m_actTime = time;

	if (!m_acts[m_current]->update(scene, time - m_actTime, deltaTime))
	{
		m_current = (m_current + 1) % m_acts.size();
		m_actTime = time;
		m_acts[m_current]->update(scene, time - m_actTime, deltaTime);
	}

	m_lastTime = time;
}

void TheaterController::setCurrentAct(uint32_t current)
{
	if (current != m_current)
	{
		m_current = current;
		m_actTime = m_lastTime;
		m_lastTime = -1.0f;
	}
}

	}
}
