/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberType.h"
#include "Resource/ResourceBundle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.resource.ResourceBundle", 0, ResourceBundle, ISerializable)

ResourceBundle::ResourceBundle(const AlignedVector< std::pair< const TypeInfo*, Guid > >& resources, bool persistent)
:	m_resources(resources)
,	m_persistent(persistent)
{
}

const AlignedVector< std::pair< const TypeInfo*, Guid > >& ResourceBundle::get() const
{
	return m_resources;
}

bool ResourceBundle::persistent() const
{
	return m_persistent;
}

void ResourceBundle::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< std::pair< const TypeInfo*, Guid >, MemberStlPair< const TypeInfo*, Guid, MemberType, Member< Guid > > >(L"resources", m_resources);
	s >> Member< bool >(L"persistent", m_persistent);
}

	}
}
