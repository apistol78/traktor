#ifndef traktor_amalgam_ScriptDebuggerStatus_H
#define traktor_amalgam_ScriptDebuggerStatus_H

#include "Core/Serialization/ISerializable.h"

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

/*! \brief Debugger status response.
 * \ingroup Amalgam
 */
class T_DLLCLASS ScriptDebuggerStatus : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerStatus();

	ScriptDebuggerStatus(bool running);

	bool isRunning() const { return m_running; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	bool m_running;
};

	}
}

#endif	// traktor_amalgam_ScriptDebuggerStatus_H
