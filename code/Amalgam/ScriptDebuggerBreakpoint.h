/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_ScriptDebuggerBreakpoint_H
#define traktor_amalgam_ScriptDebuggerBreakpoint_H

#include "Core/Guid.h"
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

/*! \brief Add or remove script breakpoint on running target.
 * \ingroup Amalgam
 */
class T_DLLCLASS ScriptDebuggerBreakpoint : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerBreakpoint();

	ScriptDebuggerBreakpoint(bool add, const Guid& scriptId, uint32_t lineNumber);

	bool shouldAdd() const { return m_add; }

	const Guid& getScriptId() const { return m_scriptId; }

	uint32_t getLineNumber() const { return m_lineNumber; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	bool m_add;
	Guid m_scriptId;
	uint32_t m_lineNumber;
};

	}
}

#endif	// traktor_amalgam_ScriptDebuggerBreakpoint_H
