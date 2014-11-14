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

/*! \brief
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

	virtual bool setBreakpoint(const Guid& scriptId, int32_t lineNumber) = 0;

	virtual bool removeBreakpoint(const Guid& scriptId, int32_t lineNumber) = 0;

	virtual bool removeAllBreakpoints(const Guid& scriptId) = 0;

	virtual bool haveBreakpoint(const Guid& scriptId, int32_t lineNumber) const = 0;

	virtual void addListener(IListener* listener) = 0;

	virtual void removeListener(IListener* listener) = 0;
};

	}
}

#endif	// traktor_script_IScriptDebuggerSessions_H
