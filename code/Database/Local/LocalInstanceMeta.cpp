#include <algorithm>
#include "Database/Local/LocalInstanceMeta.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.LocalInstanceMeta", 0, LocalInstanceMeta, ISerializable)

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

void LocalInstanceMeta::addBlob(const std::wstring& blob)
{
	m_blobs.push_back(blob);
}

void LocalInstanceMeta::removeBlob(const std::wstring& blob)
{
	std::vector< std::wstring >::iterator i = std::find(m_blobs.begin(), m_blobs.end(), blob);
	m_blobs.erase(i);
}

bool LocalInstanceMeta::haveBlob(const std::wstring& blob) const
{
	return std::find(m_blobs.begin(), m_blobs.end(), blob) != m_blobs.end();
}

const std::vector< std::wstring >& LocalInstanceMeta::getBlobs() const
{
	return m_blobs;
}

void LocalInstanceMeta::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"guid", m_guid);
	s >> Member< std::wstring >(L"primaryType", m_primaryType);
	s >> MemberStlVector< std::wstring >(L"blobs", m_blobs);
}

	}
}
