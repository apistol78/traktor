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
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! High quality timer.
 * \ingroup Core
 */
class T_DLLCLASS Timer : public Object
{
	T_RTTI_CLASS;

public:
	Timer();

	/*! Reset timer. */
	void reset();

	/*! Get number of seconds since timer started. */
	double getElapsedTime() const;

	/*! Get number of seconds since last call to this method. */
	double getDeltaTime();

private:
#if defined(__ANDROID__) || defined(__LINUX__) || defined(__RPI__)
	double m_first = 0.0;
	double m_last = 0.0;
#else
	int64_t m_frequency = 0;
	int64_t m_first = 0;
	int64_t m_last = 0;
#endif
};

}

