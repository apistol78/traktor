/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::script
{

/*! Script profiler interface.
 * \ingroup Script
 */
class T_DLLCLASS IScriptProfiler : public Object
{
	T_RTTI_CLASS;

public:
	struct IListener
	{
		virtual ~IListener() {}

		/*! Called whenever C++ invoke a function into script. */
		virtual void callEnter(const Guid& scriptId, const std::wstring& function) = 0;

		/*! Called after a C++ invoked script function returns. */
		virtual void callLeave(const Guid& scriptId, const std::wstring& function) = 0;

		/*! Called when profiler has measured duration of a script function call. */
		virtual void callMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration) = 0;
	};

	virtual void addListener(IListener* listener) = 0;

	virtual void removeListener(IListener* listener) = 0;
};

}
