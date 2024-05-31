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
#include "Runtime/Target/CommandEvent.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.CommandEvent", 0, CommandEvent, IRemoteEvent)

CommandEvent::CommandEvent(const std::wstring_view& function)
:	m_function(function)
{
}

const std::wstring& CommandEvent::getFunction() const
{
	return m_function;
}

void CommandEvent::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"function", m_function);
}

}
