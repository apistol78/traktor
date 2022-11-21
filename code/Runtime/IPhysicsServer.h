/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

class PhysicsManager;

}

namespace traktor::runtime
{

/*! Physics server.
 * \ingroup Runtime
 *
 * "Physics.Type"		- Physics manager type.
 */
class T_DLLCLASS IPhysicsServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual physics::PhysicsManager* getPhysicsManager() = 0;
};

}
