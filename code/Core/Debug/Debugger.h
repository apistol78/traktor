/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Platform specific debugger path.
 * \ingroup Core
 *
 * Used by the framework in order to invoke platform specific
 * actions when, for example, there is an assertion.
 */
class T_DLLCLASS Debugger
{
public:
	static Debugger& getInstance();

	/*! Show assert failed dialog.
	 *
	 * \param expression Failed expression.
	 * \param file Source file where assert triggered.
	 * \param line Line in source file.
	 * \param message Custom message.
	 */
	void assertionFailed(const char* const expression, const char* const file, int line, const wchar_t* const message = L"");

	/*! Check if debugger is attached. */
	bool isDebuggerAttached() const;

	/*! Break application into debugger. */
	void breakDebugger();

	/*! Report profiling event to debugger. */
	void reportEvent(const wchar_t* const text, ...);
};

}
