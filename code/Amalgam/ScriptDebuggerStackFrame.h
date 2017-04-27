/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_ScriptDebuggerStackFrame_H
#define traktor_amalgam_ScriptDebuggerStackFrame_H

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

class StackFrame;

	}

	namespace amalgam
	{

/*! \brief Response from running target when a stack frame has been captured.
 * \ingroup Amalgam
 */
class T_DLLCLASS ScriptDebuggerStackFrame : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerStackFrame();

	ScriptDebuggerStackFrame(script::StackFrame* frame);

	script::StackFrame* getFrame() const { return m_frame; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Ref< script::StackFrame > m_frame;
};

	}
}

#endif	// traktor_amalgam_ScriptDebuggerStackFrame_H
