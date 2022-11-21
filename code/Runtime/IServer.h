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

/*! System server interface.
 * \ingroup Runtime
 *
 * System servers are system pieces such
 * as rendering, input, audio etc.
 * Servers are setup using configuration values
 * from settings and must be able to be reconfigured
 * during runtime.
 */
class T_DLLCLASS IServer : public Object
{
	T_RTTI_CLASS;
};

}
