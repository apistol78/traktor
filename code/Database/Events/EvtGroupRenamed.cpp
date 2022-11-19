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
#include "Database/Events/EvtGroupRenamed.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EvtGroupRenamed", 0, EvtGroupRenamed, IEvent)

EvtGroupRenamed::EvtGroupRenamed(const std::wstring& name, const std::wstring& previousPath)
:	m_name(name)
,	m_previousPath(previousPath)
{
}

const std::wstring& EvtGroupRenamed::getName() const
{
	return m_name;
}

const std::wstring& EvtGroupRenamed::getPreviousPath() const
{
	return m_previousPath;
}

void EvtGroupRenamed::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"previousPath", m_previousPath);
}

}
