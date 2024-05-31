/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Runtime/Target/ScriptDebuggerBreakpoint.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.ScriptDebuggerBreakpoint", 0, ScriptDebuggerBreakpoint, ISerializable)

ScriptDebuggerBreakpoint::ScriptDebuggerBreakpoint(bool add, const Guid& scriptId, uint32_t lineNumber)
:	m_add(add)
,	m_scriptId(scriptId)
,	m_lineNumber(lineNumber)
{
}

void ScriptDebuggerBreakpoint::serialize(ISerializer& s)
{
	s >> Member< bool >(L"add", m_add);
	s >> Member< Guid >(L"scriptId", m_scriptId);
	s >> Member< uint32_t >(L"lineNumber", m_lineNumber);
}

}
