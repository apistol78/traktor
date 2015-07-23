#ifndef traktor_amalgam_UpdateControl_H
#define traktor_amalgam_UpdateControl_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Update control.
 * \ingroup Amalgam
 *
 * Application update control interface, as
 * applications can become paused or similar
 * either internally, from application itself,
 * or externally, window minimized, this
 * interface provide a two-way method of
 * handling update scenarios.
 */
class T_DLLCLASS UpdateControl : public Object
{
	T_RTTI_CLASS;

public:
	UpdateControl();

	void setPause(bool pause) { m_pause = pause; }

	bool getPause() const { return m_pause; }

	void setTimeScale(float timeScale) { m_timeScale = timeScale; }

	float getTimeScale() const { return m_timeScale; }

	void setSimulationFrequency(float simulationFrequency) { m_simulationFrequency = simulationFrequency; }

	float getSimulationFrequency() const { return m_simulationFrequency; }

private:
	friend class Application;

	bool m_pause;
	float m_timeScale;
	float m_simulationFrequency;
};

	}
}

#endif	// traktor_amalgam_UpdateControl_H
