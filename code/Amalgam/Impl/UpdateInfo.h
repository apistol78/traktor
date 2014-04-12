#ifndef traktor_amalgam_UpdateInfo_H
#define traktor_amalgam_UpdateInfo_H

#include "Core/Math/MathUtils.h"
#include "Amalgam/IUpdateInfo.h"

namespace traktor
{
	namespace amalgam
	{

/*! \brief Update information. */
class UpdateInfo : public IUpdateInfo
{
	T_RTTI_CLASS;

public:
	float m_totalTime;
	float m_stateTime;
	float m_simulationTime;
	float m_simulationDeltaTime;
	int32_t m_simulationFrequency;
	float m_frameDeltaTime;
	uint64_t m_frame;
	bool m_runningSlow;

	UpdateInfo()
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

	virtual float getTotalTime() const { return m_totalTime; }

	virtual float getStateTime() const { return m_stateTime; }

	virtual float getSimulationTime() const { return m_simulationTime; }

	virtual float getSimulationDeltaTime() const { return m_simulationDeltaTime; }

	virtual int32_t getSimulationFrequency() const { return m_simulationFrequency; }

	virtual float getFrameDeltaTime() const { return m_frameDeltaTime; }

	virtual uint64_t getFrame() const { return m_frame; }

	virtual float getInterval() const { return clamp((m_stateTime - m_simulationTime + m_simulationDeltaTime) / m_simulationDeltaTime, 0.0f, 1.0f); }

	virtual bool isRunningSlow() const { return m_runningSlow; }
};

	}
}

#endif	// traktor_amalgam_UpdateInfo_H
