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
#include "Runtime/IAudioServer.h"
#include "Runtime/IInputServer.h"
#include "Runtime/IOnlineServer.h"
#include "Runtime/IPhysicsServer.h"
#include "Runtime/IRenderServer.h"
#include "Runtime/IResourceServer.h"
#include "Runtime/IScriptServer.h"
#include "Runtime/IWorldServer.h"
#include "Runtime/Types.h"
#include "Runtime/UpdateControl.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

}

namespace traktor::db
{

class Database;

}

namespace traktor::runtime
{

/*! Runtime environment.
 * \ingroup Runtime
 *
 * This class represent the runtime environment.
 * It's a single accessors for applications to reach
 * various system objects.
 */
class T_DLLCLASS IEnvironment : public Object
{
	T_RTTI_CLASS;

public:
	virtual db::Database* getDatabase() = 0;

	virtual UpdateControl* getControl() = 0;

	/*! \name Server accessors. */
	/*! \{ */

	virtual IAudioServer* getAudio() = 0;

	virtual IInputServer* getInput() = 0;

	virtual IOnlineServer* getOnline() = 0;

	virtual IPhysicsServer* getPhysics() = 0;

	virtual IRenderServer* getRender() = 0;

	virtual IResourceServer* getResource() = 0;

	virtual IScriptServer* getScript() = 0;

	virtual IWorldServer* getWorld() = 0;

	/*! \} */

	/*! \name Settings. */
	/*! \{ */

	virtual PropertyGroup* getSettings() = 0;

	virtual bool reconfigure() = 0;

	/*! \} */
};

}
