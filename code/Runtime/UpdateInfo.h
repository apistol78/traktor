/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Math/MathUtils.h"

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

/*! Update information.
 * \ingroup Runtime
 *
 * Update information provide time measurements
 * to user applications.
 */
class T_DLLCLASS UpdateInfo : public Object
{
	T_RTTI_CLASS;

public:
	/*! Get total application time.
	 *
	 * \return Time in seconds.
	 */
	float getTotalTime() const { return m_totalTime; }

	/*! Get real time in current state.
	 *
	 * \return Time in seconds.
	 */
	float getStateTime() const { return m_stateTime; }

	/*! Get simulation time in current state.
	 *
	 * \note Simulation time are updated in discreet
	 * time steps thus might not be same as state time.
	 * Can be used to derive frame interpolation offset
	 * as \"offset = simulation time - state time\"
	 *
	 * \return Time in seconds.
	 */
	float getSimulationTime() const { return m_simulationTime; }

	/*! Get simulation delta time.
	 *
	 * Simulation delta time is a fixed value
	 * defined at the beginning of the application.
	 *
	 * \return Delta time in seconds.
	 */
	float getSimulationDeltaTime() const { return m_simulationDeltaTime; }

	/*! Get simulation frequency.
	 *
	 * \return Frequency in hertz.
	 */
	float getSimulationFrequency() const { return m_simulationFrequency; }

	/*! Get frame delta time.
	 *
	 * Frame delta time varies depending on current
	 * frame-rate.
	 *
	 * \return Delta time in seconds.
	 */
	float getFrameDeltaTime() const { return m_frameDeltaTime; }

	/*! Get frame count.
	 *
	 * \return Frame count since beginning of application.
	 */
	uint64_t getFrame() const { return m_frame; }

	/*! Get interval fraction.
	 *
	 * \return Interval fraction.
	 */
	float getInterval() const { return (m_stateTime - m_simulationTime) / m_simulationDeltaTime; }

	/*! If system is detected as being too slow.
	 *
	 * \return True if system incapable of sustaining a stable frame rate.
	 */
	bool isRunningSlow() const { return m_runningSlow; }

private:
	friend class Application;

	float m_totalTime = 0.0f;
	float m_stateTime = 0.0f;
	float m_simulationTime = 0.0f;
	float m_simulationDeltaTime = 0.0f;
	float m_simulationFrequency = 0.0f;
	float m_frameDeltaTime = 0.0f;
	uint64_t m_frame = 0;
	bool m_runningSlow = false;
};

	}
}

