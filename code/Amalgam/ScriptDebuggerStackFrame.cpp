/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/ScriptDebuggerStackFrame.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Script/StackFrame.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.ScriptDebuggerStackFrame", 0, ScriptDebuggerStackFrame, ISerializable)

ScriptDebuggerStackFrame::ScriptDebuggerStackFrame()
{
}

ScriptDebuggerStackFrame::ScriptDebuggerStackFrame(script::StackFrame* frame)
:	m_frame(frame)
{
}

void ScriptDebuggerStackFrame::serialize(ISerializer& s)
{
	s >> MemberRef< script::StackFrame >(L"frame", m_frame);
}

	}
}
