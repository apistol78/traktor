#ifndef traktor_amalgam_ScriptDebuggerLocals_H
#define traktor_amalgam_ScriptDebuggerLocals_H

#include "Core/RefArray.h"
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
	namespace script
	{

class Local;

	}

	namespace amalgam
	{

/*! \brief Response from running target when locals has been captured.
 * \ingroup Amalgam
 */
class T_DLLCLASS ScriptDebuggerLocals : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerLocals();

	ScriptDebuggerLocals(const RefArray< script::Local >& locals);

	const RefArray< script::Local >& getLocals() const { return m_locals; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	RefArray< script::Local > m_locals;
};

	}
}

#endif	// traktor_amalgam_ScriptDebuggerLocals_H
