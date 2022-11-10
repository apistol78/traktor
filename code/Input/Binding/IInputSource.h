/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputSystem;
class InputValueSet;

/*! Abstract input source.
 * \ingroup Input
 */
class T_DLLCLASS IInputSource : public Object
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const = 0;

	virtual void prepare(float T, float dT) = 0;

	/*! Read value from input source.
	 *
	 * \note
	 * Should return NaN if unable to read value
	 * from source as it's possibly used by input
	 * graph to mask features.
	 *
	 * \param T Simulation time.
	 * \param dT Delta time since last read.
	 * \return Input source value.
	 */
	virtual float read(float T, float dT) = 0;
};

	}
}

