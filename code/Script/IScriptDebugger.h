#ifndef traktor_script_IScriptDebugger_H
#define traktor_script_IScriptDebugger_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Guid;

	namespace script
	{

class StackFrame;

/*! \brief Script debugger
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

	virtual bool setBreakpoint(const Guid& scriptId, int32_t lineNumber) = 0;

	virtual bool removeBreakpoint(const Guid& scriptId, int32_t lineNumber) = 0;

	virtual Ref< StackFrame > captureStackFrame(uint32_t depth) = 0;

	virtual bool isRunning() const = 0;

	virtual bool actionBreak() = 0;

	virtual bool actionContinue() = 0;

	virtual bool actionStepInto() = 0;

	virtual bool actionStepOver() = 0;

	virtual void addListener(IListener* listener) = 0;

	virtual void removeListener(IListener* listener) = 0;
};

	}
}

#endif	// traktor_script_IScriptDebugger_H
