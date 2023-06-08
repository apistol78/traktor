/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include <map>
#include <set>
#include "Core/Ref.h"
#include "Script/Editor/IScriptDebuggerSessions.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::script
{

/*!
 * \ingroup script
 */
class T_DLLCLASS ScriptDebuggerSessions : public IScriptDebuggerSessions
{
	T_RTTI_CLASS;

public:
	virtual void beginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) override final;

	virtual void endSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) override final;

	virtual bool setBreakpoint(const Guid& scriptId, int32_t lineNumber) override final;

	virtual bool removeBreakpoint(const Guid& scriptId, int32_t lineNumber) override final;

	virtual bool removeAllBreakpoints(const Guid& scriptId) override final;

	virtual bool haveBreakpoint(const Guid& scriptId, int32_t lineNumber) const override final;

	virtual void addListener(IListener* listener) override final;

	virtual void removeListener(IListener* listener) override final;

private:
	struct Session
	{
		Ref< IScriptDebugger > debugger;
		Ref< IScriptProfiler > profiler;
	};
	std::list< Session > m_sessions;
	std::map< int32_t, std::set< Guid > > m_breakpoints;
	std::list< IListener* > m_listeners;
};

}
