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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::runtime
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

	void setTimeScale(double timeScale) { m_timeScale = timeScale; }

	double getTimeScale() const { return m_timeScale; }

	void setSimulationFrequency(double simulationFrequency) { m_simulationFrequency = simulationFrequency; }

	double getSimulationFrequency() const { return m_simulationFrequency; }

private:
	friend class Application;

	bool m_pause = false;
	double m_timeScale = 1.0;
	double m_simulationFrequency = 60.0;
};

}
