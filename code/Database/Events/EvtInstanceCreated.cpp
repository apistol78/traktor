/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Database/Events/EvtInstanceCreated.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EvtInstanceCreated", 0, EvtInstanceCreated, EvtInstance)

EvtInstanceCreated::EvtInstanceCreated(const std::wstring& groupPath, const Guid& instanceGuid)
:	EvtInstance(instanceGuid)
,	m_groupPath(groupPath)
{
}

const std::wstring& EvtInstanceCreated::getGroupPath() const
{
	return m_groupPath;
}

void EvtInstanceCreated::serialize(ISerializer& s)
{
	EvtInstance::serialize(s);
	s >> Member< std::wstring >(L"groupPath", m_groupPath);
}

}
