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
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::runtime
{

class IEnvironment;
class IStateManager;

/*! Runtime application.
 * \ingroup Runtime
 *
 * This interface represent the running
 * instance of the application.
 */
class T_DLLCLASS IApplication : public Object
{
	T_RTTI_CLASS;

public:
	/*! Get runtime environment.
	 *
	 * \return Runtime environment.
	 */
	virtual IEnvironment* getEnvironment() = 0;

	/*! Get state manager.
	 *
	 * \return State manager.
	 */
	virtual IStateManager* getStateManager() = 0;
};

}
