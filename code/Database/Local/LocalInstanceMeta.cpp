/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
// #include <algorithm>
#include "Database/Local/LocalInstanceMeta.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberSmallSet.h"

namespace traktor::db
{
	namespace
	{

struct Blob
{
	std::wstring name;
	std::wstring hash;

	void serialize(ISerializer& s)
	{
		s >> Member< std::wstring >(L"name", name);
		s >> Member< std::wstring >(L"hash", hash);
	}
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.LocalInstanceMeta", 2, LocalInstanceMeta, ISerializable)

LocalInstanceMeta::LocalInstanceMeta(const Guid& guid, const std::wstring& primaryType)
:	m_guid(guid)
,	m_primaryType(primaryType)
{
}

void LocalInstanceMeta::setGuid(const Guid& guid)
{
	m_guid = guid;
}

const Guid& LocalInstanceMeta::getGuid() const
{
	return m_guid;
}

void LocalInstanceMeta::setPrimaryType(const std::wstring& primaryType)
{
	m_primaryType = primaryType;
}

const std::wstring& LocalInstanceMeta::getPrimaryType() const
{
	return m_primaryType;
}

void LocalInstanceMeta::setBlob(const std::wstring& name)
{
	m_blobs.insert(name);
}

void LocalInstanceMeta::removeBlob(const std::wstring& name)
{
	m_blobs.erase(name);
}

void LocalInstanceMeta::removeAllBlobs()
{
	m_blobs.clear();
}

bool LocalInstanceMeta::haveBlob(const std::wstring& name) const
{
	return m_blobs.find(name) != m_blobs.end();
}

const SmallSet< std::wstring >& LocalInstanceMeta::getBlobs() const
{
	return m_blobs;
}

void LocalInstanceMeta::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"guid", m_guid);
	s >> Member< std::wstring >(L"primaryType", m_primaryType);

	if (s.getVersion< LocalInstanceMeta >() >= 2)
	{
		s >> MemberSmallSet< std::wstring >(L"blobs", m_blobs);
	}
	else if (s.getVersion< LocalInstanceMeta >() >= 1)
	{
		AlignedVector< Blob > blobs;
		s >> MemberAlignedVector< Blob, MemberComposite< Blob > >(L"blobs", blobs);
		for (const auto& blob : blobs)
			m_blobs.insert(blob.name);		
	}
	else
	{
		AlignedVector< std::wstring > blobs;
		s >> MemberAlignedVector< std::wstring >(L"blobs", blobs);
		for (const auto& blob : blobs)
			m_blobs.insert(blob);
	}
}

}
