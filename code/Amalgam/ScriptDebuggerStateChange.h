/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_ScriptDebuggerStateChange_H
#define traktor_amalgam_ScriptDebuggerStateChange_H

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

/*! \brief Event from target when debug state has changed.
 * \ingroup Amalgam
 */
class T_DLLCLASS ScriptDebuggerStateChange : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerStateChange();

	ScriptDebuggerStateChange(bool running);

	bool isRunning() const { return m_running;  }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	bool m_running;
};

	}
}

#endif	// traktor_amalgam_ScriptDebuggerStateChange_H
