/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_UpdateInfo_H
#define traktor_amalgam_UpdateInfo_H

#include "Core/Object.h"
#include "Core/Math/MathUtils.h"

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

/*! \brief Update information.
 * \ingroup Amalgam
 *
 * Update information provide time measurements
 * to user applications.
 */
class T_DLLCLASS UpdateInfo : public Object
{
	T_RTTI_CLASS;

public:
	UpdateInfo();

	/*! \brief Get total application time.
	 *
	 * \return Time in seconds.
	 */
	float getTotalTime() const { return m_totalTime; }

	/*! \brief Get real time in current state.
	 *
	 * \return Time in seconds.
	 */
	float getStateTime() const { return m_stateTime; }

	/*! \brief Get simulation time in current state.
	 *
	 * \note Simulation time are updated in discreet
	 * time steps thus might not be same as state time.
	 * Can be used to derive frame interpolation offset
	 * as \"offset = simulation time - state time\"
	 *
	 * \return Time in seconds.
	 */
	float getSimulationTime() const { return m_simulationTime; }

	/*! \brief Get simulation delta time.
	 *
	 * Simulation delta time is a fixed value
	 * defined at the beginning of the application.
	 *
	 * \return Delta time in seconds.
	 */
	float getSimulationDeltaTime() const { return m_simulationDeltaTime; }

	/*! \brief Get simulation frequency.
	 *
	 * \return Frequency in hertz.
	 */
	int32_t getSimulationFrequency() const { return m_simulationFrequency; }

	/*! \brief Get frame delta time.
	 *
	 * Frame delta time varies depending on current
	 * frame-rate.
	 *
	 * \return Delta time in seconds.
	 */
	float getFrameDeltaTime() const { return m_frameDeltaTime; }

	/*! \brief Get frame count.
	 *
	 * \return Frame count since beginning of application.
	 */
	uint64_t getFrame() const { return m_frame; }

	/*! \brief Get interval fraction.
	 *
	 * I = 1 - (Tsim - Tstate) / dTsim
	 *
	 * \return Interval fraction.
	 */
	float getInterval() const { return clamp((m_stateTime - m_simulationTime) / m_simulationDeltaTime, 0.0f, 1.0f); }

	/*! \brief If system is detected as being too slow.
	 *
	 * \return True if system incapable of sustaining a stable frame rate.
	 */
	bool isRunningSlow() const { return m_runningSlow; }

private:
	friend class Application;

	float m_totalTime;
	float m_stateTime;
	float m_simulationTime;
	float m_simulationDeltaTime;
	int32_t m_simulationFrequency;
	float m_frameDeltaTime;
	uint64_t m_frame;
	bool m_runningSlow;
};

	}
}

#endif	// traktor_amalgam_UpdateInfo_H
