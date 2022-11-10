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
#include "Core/Serialization/MemberRef.h"
#include "Net/Discovery/NetworkService.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.net.NetworkService", 1, NetworkService, IService)

NetworkService::NetworkService(
	const std::wstring& type,
	const PropertyGroup* properties
)
:	m_type(type)
,	m_properties(properties)
{
}

const std::wstring& NetworkService::getType() const
{
	return m_type;
}

const PropertyGroup* NetworkService::getProperties() const
{
	return m_properties;
}

void NetworkService::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"type", m_type);
	s >> MemberRef< const PropertyGroup >(L"properties", m_properties);
}

	}
}
