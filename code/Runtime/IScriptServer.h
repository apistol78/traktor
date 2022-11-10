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
	namespace script
	{

class IScriptContext;
class IScriptManager;

	}

	namespace runtime
	{

/*! Script server.
 * \ingroup Runtime
 *
 * "Script.Library"	- Script library.
 * "Script.Type"	- Script manager type.
 */
class T_DLLCLASS IScriptServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual script::IScriptManager* getScriptManager() = 0;

	virtual script::IScriptContext* getScriptContext() = 0;
};

	}
}

