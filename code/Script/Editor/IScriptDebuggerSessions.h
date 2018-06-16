/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_IScriptDebuggerSessions_H
#define traktor_script_IScriptDebuggerSessions_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Guid;

	namespace script
	{

class IScriptDebugger;
class IScriptProfiler;

/*! \brief Debugger sessions interface.
 * \ingroup Script
 */
class T_DLLCLASS IScriptDebuggerSessions : public Object
{
	T_RTTI_CLASS;

public:
	struct IListener
	{
		virtual void notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) = 0;

		virtual void notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) = 0;

		virtual void notifySetBreakpoint(const Guid& scriptId, int32_t lineNumber) = 0;

		virtual void notifyRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber) = 0;
	};

	/*! \brief Begin new debugging session.
	 *
	 * \param scriptDebugger Script debugger.
	 * \param scriptProfiler Script profiler.
	 */
	virtual void beginSession(script::IScriptDebugger* scriptDebugger, script::IScriptProfiler* scriptProfiler) = 0;

	/*! \brief End debugging session.
	 *
	 * \param scriptDebugger Script debugger.
	 * \param scriptProfiler Script profiler.
	 */
	virtual void endSession(script::IScriptDebugger* scriptDebugger, script::IScriptProfiler* scriptProfiler) = 0;

	/*! \brief Activate new breakpoint.
	 *
	 * \param scriptId Script identifier.
	 * \param lineNumber Line within script to associate with breakpoint.
	 * \return True if breakpoint was set.
	 */
	virtual bool setBreakpoint(const Guid& scriptId, int32_t lineNumber) = 0;

	/*! \brief Remove breakpoint.
	 *
	 * \param scriptId Script identifier.
	 * \param lineNumber Line within script to associate with breakpoint.
	 * \return True if breakpoint was removed.
	 */
	virtual bool removeBreakpoint(const Guid& scriptId, int32_t lineNumber) = 0;

	/*! \brief Remove all breakpoints from script.
	 *
	 * \param scriptId Script identifier.
	 * \return True if all breakpoints was removed.
	 */
	virtual bool removeAllBreakpoints(const Guid& scriptId) = 0;

	/*! \brief Check if breakpoint is set.
	 *
	 * \param scriptId Script identifier.
	 * \param lineNumber Line within script to check for breakpoint.
	 * \return True if breakpoint is set.
	 */
	virtual bool haveBreakpoint(const Guid& scriptId, int32_t lineNumber) const = 0;

	/*! \brief Add session listener.
	 *
	 * \param listener Listener implementation.
	 */
	virtual void addListener(IListener* listener) = 0;

	/*! \brief Remove session listener.
	 *
	 * \param listener Listener to remove.
	 */
	virtual void removeListener(IListener* listener) = 0;
};

	}
}

#endif	// traktor_script_IScriptDebuggerSessions_H
