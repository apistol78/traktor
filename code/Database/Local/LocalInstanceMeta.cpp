#include <algorithm>
#include "Database/Local/LocalInstanceMeta.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.LocalInstanceMeta", 1, LocalInstanceMeta, ISerializable)

LocalInstanceMeta::LocalInstanceMeta()
{
}

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

void LocalInstanceMeta::setBlob(const std::wstring& name, const std::wstring& hash)
{
	auto it = std::find_if(m_blobs.begin(), m_blobs.end(), [&](const Blob& blob) { return blob.name == name; });
	if (it != m_blobs.end())
		it->hash = hash;
	else
		m_blobs.push_back({ name, hash });
}

void LocalInstanceMeta::removeBlob(const std::wstring& name)
{
	auto it = std::find_if(m_blobs.begin(), m_blobs.end(), [&](const Blob& blob) { return blob.name == name; });
	if (it != m_blobs.end())
		m_blobs.erase(it);
}

bool LocalInstanceMeta::haveBlob(const std::wstring& name) const
{
	auto it = std::find_if(m_blobs.begin(), m_blobs.end(), [&](const Blob& blob) { return blob.name == name; });
	return it != m_blobs.end();
}

const AlignedVector< LocalInstanceMeta::Blob >& LocalInstanceMeta::getBlobs() const
{
	return m_blobs;
}

void LocalInstanceMeta::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"guid", m_guid);
	s >> Member< std::wstring >(L"primaryType", m_primaryType);
	if (s.getVersion< LocalInstanceMeta >() >= 1)
		s >> MemberAlignedVector< Blob, MemberComposite< Blob > >(L"blobs", m_blobs);
	else
	{
		AlignedVector< std::wstring > blobs;
		s >> MemberAlignedVector< std::wstring >(L"blobs", blobs);
		for (const auto& blob : blobs)
			m_blobs.push_back({ blob, L"" });
	}
}

void LocalInstanceMeta::Blob::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< std::wstring >(L"hash", hash);
}

	}
}
