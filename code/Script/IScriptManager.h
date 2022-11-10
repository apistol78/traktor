/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Class/IRuntimeClassRegistrar.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

	namespace script
	{

class IErrorCallback;
class IScriptBlob;
class IScriptContext;
class IScriptDebugger;
class IScriptProfiler;

/*! Script runtime statistics.
 * \ingroup Script
 */
struct ScriptStatistics
{
	uint32_t memoryUsage;
};

/*! Script manager.
 * \ingroup Script
 *
 * A script manager is the major communicator
 * with the desired scripting environment.
 * It's purpose is to handle class registration
 * and context creations.
 * Each context should have at least the classes
 * registered up until the time of creation accessible.
 */
class T_DLLCLASS IScriptManager
:	public Object
,	public IRuntimeClassRegistrar
{
	T_RTTI_CLASS;

public:
	/*! Destroy script manager. */
	virtual void destroy() = 0;

	/*! Create script context.
	 *
	 * \param strict Strict global variable declaration required.
	 * \return Script context instance.
	 */
	virtual Ref< IScriptContext > createContext(bool strict) = 0;

	/*! Create debugger.
	 *
	 * \return Debugger instance.
	 */
	virtual Ref< IScriptDebugger > createDebugger() = 0;

	/*! Create profiler.
	 *
	 * \return Profiler instance.
	 */
	virtual Ref< IScriptProfiler > createProfiler() = 0;

	/*! Collect garbage.
	 *
	 * This is exposed in order to make some scripting languages which
	 * rely on garbage collection behave more sane in real-time applications
	 * such as games where we cannot afford both the memory required nor the
	 * time spent in normal garbage collection cycles.
	 * Thus we must be able to single step garbage collection at a
	 * high frequency in order to keep the heap as small as possible.
	 *
	 * If this isn't called the normal garbage collection frequency
	 * should be used as dictated by the scripting language.
	 */
	virtual void collectGarbage(bool full) = 0;

	/*! */
	virtual void getStatistics(ScriptStatistics& outStatistics) const = 0;
};

	}
}

