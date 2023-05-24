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

namespace traktor::runtime
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
	double getTotalTime() const { return m_totalTime; }

	/*! Get real time in current state.
	 *
	 * \return Time in seconds.
	 */
	double getStateTime() const { return m_stateTime; }

	/*! Get simulation time in current state.
	 *
	 * \note Simulation time are updated in discreet
	 * time steps thus might not be same as state time.
	 * Can be used to derive frame interpolation offset
	 * as \"offset = simulation time - state time\"
	 *
	 * \return Time in seconds.
	 */
	double getSimulationTime() const { return m_simulationTime; }

	/*! Get simulation delta time.
	 *
	 * Simulation delta time is a fixed value
	 * defined at the beginning of the application.
	 *
	 * \return Delta time in seconds.
	 */
	double getSimulationDeltaTime() const { return m_simulationDeltaTime; }

	/*! Get simulation frequency.
	 *
	 * \return Frequency in hertz.
	 */
	double getSimulationFrequency() const { return m_simulationFrequency; }

	/*! Get frame delta time.
	 *
	 * Frame delta time varies depending on current
	 * frame-rate.
	 *
	 * \return Delta time in seconds.
	 */
	double getFrameDeltaTime() const { return m_frameDeltaTime; }

	/*! Get frame count.
	 *
	 * \return Frame count since beginning of application.
	 */
	uint64_t getFrame() const { return m_frame; }

	/*! Get interval fraction.
	 *
	 * \return Interval fraction.
	 */
	float getInterval() const { return m_interval; }

private:
	friend class Application;

	double m_totalTime = 0.0;
	double m_stateTime = 0.0;
	double m_simulationTime = 0.0;
	double m_simulationDeltaTime = 0.0;
	double m_simulationFrequency = 0.0;
	double m_frameDeltaTime = 0.0;
	float m_interval = 0.0f;
	uint64_t m_frame = 0;
};

}
