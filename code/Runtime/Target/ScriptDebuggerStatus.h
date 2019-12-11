#pragma once

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
	namespace runtime
	{

/*! Debugger status response.
 * \ingroup Runtime
 */
class T_DLLCLASS ScriptDebuggerStatus : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerStatus();

	ScriptDebuggerStatus(bool running);

	bool isRunning() const { return m_running; }

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_running;
};

	}
}

