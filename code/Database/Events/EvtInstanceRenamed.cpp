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
#include "Database/Events/EvtInstanceRenamed.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EvtInstanceRenamed", 0, EvtInstanceRenamed, EvtInstance)

EvtInstanceRenamed::EvtInstanceRenamed(const Guid& instanceGuid, const std::wstring& previousName)
:	EvtInstance(instanceGuid)
,	m_previousName(previousName)
{
}

const std::wstring& EvtInstanceRenamed::getPreviousName() const
{
	return m_previousName;
}

void EvtInstanceRenamed::serialize(ISerializer& s)
{
	EvtInstance::serialize(s);
	s >> Member< std::wstring >(L"previousName", m_previousName);
}

}
