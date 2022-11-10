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
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class Body;

/*! Joint base class.
 * \ingroup Physics
 */
class T_DLLCLASS Joint : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	/*! Get first constraint body. */
	virtual Body* getBody1() = 0;

	/*! Get second constraint body.
	 *
	 * \note
	 * This return null if first body is constrained
	 * to world.
	 */
	virtual Body* getBody2() = 0;

	/*! \brief
	 */
	virtual void* getInternal() = 0;
};

	}
}

