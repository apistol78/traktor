/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::script
{

class Variable;
class StackFrame;

/*! Script debugger
 * \ingroup Script
 */
class T_DLLCLASS IScriptDebugger : public Object
{
	T_RTTI_CLASS;

public:
	struct IListener
	{
		virtual ~IListener() {}

		virtual void debugeeStateChange(IScriptDebugger* scriptDebugger) = 0;
	};

	virtual bool setBreakpoint(const std::wstring& fileName, int32_t lineNumber) = 0;

	virtual bool removeBreakpoint(const std::wstring& fileName, int32_t lineNumber) = 0;

	virtual bool captureStackFrame(uint32_t depth, Ref< StackFrame >& outStackFrame) = 0;

	virtual bool captureLocals(uint32_t depth, RefArray< Variable >& outLocals) = 0;

	virtual bool captureObject(uint32_t object, RefArray< Variable >& outMembers) = 0;

	virtual bool captureBreadcrumbs(AlignedVector< uint32_t >& outBreadcrumbs) = 0;

	virtual bool isRunning() const = 0;

	virtual bool actionBreak() = 0;

	virtual bool actionContinue() = 0;

	virtual bool actionStepInto() = 0;

	virtual bool actionStepOver() = 0;

	virtual void addListener(IListener* listener) = 0;

	virtual void removeListener(IListener* listener) = 0;
};

}
