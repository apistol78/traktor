/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Target/TargetID.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TargetID", 0, TargetID, ISerializable)

TargetID::TargetID(const Guid& id, const std::wstring& name)
:	m_id(id)
,	m_name(name)
{
}

const Guid& TargetID::getId() const
{
	return m_id;
}

const std::wstring& TargetID::getName() const
{
	return m_name;
}

void TargetID::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"id", m_id);
	s >> Member< std::wstring >(L"name", m_name);
}

}
