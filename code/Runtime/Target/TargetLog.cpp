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
#include "Runtime/Target/TargetLog.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TargetLog", 0, TargetLog, ISerializable)

TargetLog::TargetLog(uint32_t threadId, int32_t level, const std::wstring& text)
:	m_threadId(threadId)
,	m_level(level)
,	m_text(text)
{
}

void TargetLog::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"threadId", m_threadId);
	s >> Member< int32_t >(L"level", m_level);
	s >> Member< std::wstring >(L"text", m_text);
}

}
