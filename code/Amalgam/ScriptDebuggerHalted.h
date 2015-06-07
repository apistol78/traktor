#ifndef traktor_amalgam_ScriptDebuggerHalted_H
#define traktor_amalgam_ScriptDebuggerHalted_H

#include "Core/Serialization/ISerializable.h"
#include "Script/CallStack.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Response from running target when debugger has been halted.
 * \ingroup Amalgam
 */
class T_DLLCLASS ScriptDebuggerHalted : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerHalted();

	ScriptDebuggerHalted(const script::CallStack& callStack);

	const script::CallStack& getCallStack() const { return m_callStack; }

	virtual void serialize(ISerializer& s);

private:
	script::CallStack m_callStack;
};

	}
}

#endif	// traktor_amalgam_ScriptDebuggerHalted_H
