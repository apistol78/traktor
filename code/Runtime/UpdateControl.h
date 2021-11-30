#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

/*! Update control.
 * \ingroup Runtime
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
	void setPause(bool pause) { m_pause = pause; }

	bool getPause() const { return m_pause; }

	void setTimeScale(float timeScale) { m_timeScale = timeScale; }

	float getTimeScale() const { return m_timeScale; }

	void setSimulationFrequency(float simulationFrequency) { m_simulationFrequency = simulationFrequency; }

	float getSimulationFrequency() const { return m_simulationFrequency; }

private:
	friend class Application;

	bool m_pause = false;
	float m_timeScale = 1.0f;
	float m_simulationFrequency = 60.0f;
};

	}
}

