/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Debug/Debugger.h"
#include "Core/Log/Log.h"

#define T_CAPTURE_ASSERT(condition, message) \
	if (!(condition)) { \
		log::error << L"Render error: " << message << Endl; \
		Debugger::getInstance().breakDebugger(); \
	}

#if 0
#	define T_CAPTURE_TRACE(method) \
		log::debug << method << Endl;
#else
#	define T_CAPTURE_TRACE(method)
#endif

