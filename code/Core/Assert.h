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

/*! \ingroup Core */
//@{

#if defined(_DEBUG)
#	define T_ASSERT(expression) \
		(void)((!!(expression)) || (traktor::Debugger::getInstance().assertionFailed(#expression, __FILE__, __LINE__), 0));

#	define T_ASSERT_M(expression, message) \
		(void)((!!(expression)) || (traktor::Debugger::getInstance().assertionFailed(#expression, __FILE__, __LINE__, message), 0));

#	define T_BREAKPOINT \
		traktor::Debugger::getInstance().breakDebugger();
#else
#	define T_ASSERT(expression)
#	define T_ASSERT_M(expression, message)
#	define T_BREAKPOINT
#endif

#define T_FATAL_ASSERT(expression) \
	(void)((!!(expression)) || (traktor::Debugger::getInstance().assertionFailed(#expression, __FILE__, __LINE__), 0));

#define T_FATAL_ASSERT_M(expression, message) \
	(void)((!!(expression)) || (traktor::Debugger::getInstance().assertionFailed(#expression, __FILE__, __LINE__, message), 0));

#define T_FATAL_ERROR \
	traktor::Debugger::getInstance().breakDebugger();

//@}
