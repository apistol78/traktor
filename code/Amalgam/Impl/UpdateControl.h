#ifndef traktor_amalgam_UpdateControl_H
#define traktor_amalgam_UpdateControl_H

#include "Amalgam/IUpdateControl.h"

namespace traktor
{
	namespace amalgam
	{

/*! \brief Update control. */
class UpdateControl : public IUpdateControl
{
	T_RTTI_CLASS;

public:
	bool m_pause;
	float m_timeScale;
	float m_simulationFrequency;

	UpdateControl()
	:	m_pause(false)
	,	m_timeScale(1.0f)
	,	m_simulationFrequency(60.0f)
	{
	}

	virtual void setPause(bool pause) { m_pause = pause; }

	virtual bool getPause() const { return m_pause; }

	virtual void setTimeScale(float timeScale) { m_timeScale = timeScale; }

	virtual float getTimeScale() const { return m_timeScale; }

	virtual void setSimulationFrequency(float simulationFrequency) { m_simulationFrequency = simulationFrequency; }

	virtual float getSimulationFrequency() const { return m_simulationFrequency; }
};

	}
}

#endif	// traktor_amalgam_UpdateControl_H
