/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Runtime/Target/ScriptDebuggerStackFrame.h"
#include "Script/StackFrame.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.ScriptDebuggerStackFrame", 0, ScriptDebuggerStackFrame, ISerializable)

ScriptDebuggerStackFrame::ScriptDebuggerStackFrame(script::StackFrame* frame)
:	m_frame(frame)
{
}

void ScriptDebuggerStackFrame::serialize(ISerializer& s)
{
	s >> MemberRef< script::StackFrame >(L"frame", m_frame);
}

}
