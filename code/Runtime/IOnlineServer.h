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

namespace traktor
{
	namespace online
	{

class ISessionManager;

	}

	namespace runtime
	{

/*! Online server.
 * \ingroup Runtime
 */
class T_DLLCLASS IOnlineServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual online::ISessionManager* getSessionManager() = 0;
};

	}
}

