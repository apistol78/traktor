#pragma once

#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class StackFrame;

	}

	namespace runtime
	{

/*! Response from running target when a stack frame has been captured.
 * \ingroup Runtime
 */
class T_DLLCLASS ScriptDebuggerStackFrame : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerStackFrame() = default;

	explicit ScriptDebuggerStackFrame(script::StackFrame* frame);

	script::StackFrame* getFrame() const { return m_frame; }

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< script::StackFrame > m_frame;
};

	}
}

