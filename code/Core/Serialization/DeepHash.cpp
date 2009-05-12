#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Io/DynamicMemoryStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.DeepHash", DeepHash, Object)

DeepHash::DeepHash(const Serializable* object)
{
	std::vector< uint8_t > copy;

	// Create flat binary copy of object.
	DynamicMemoryStream stream(copy, false, true);
	BinarySerializer(&stream).writeObject(object);

	// Calculate MD5 checksum of binary object.
	m_md5.begin();
	m_md5.feed(&copy[0], uint32_t(copy.size()));
	m_md5.end();
}

const MD5& DeepHash::getMD5() const
{
	return m_md5;
}

bool DeepHash::operator == (const DeepHash& hash) const
{
	return m_md5 == hash.m_md5;
}

bool DeepHash::operator != (const DeepHash& hash) const
{
	return m_md5 != hash.m_md5;
}

bool DeepHash::operator == (const DeepHash* hash) const
{
	return m_md5 == hash->m_md5;
}

bool DeepHash::operator != (const DeepHash* hash) const
{
	return m_md5 != hash->m_md5;
}

bool DeepHash::operator == (const Serializable* object) const
{
	DeepHash hash(object);
	return m_md5 == hash.m_md5;
}

bool DeepHash::operator != (const Serializable* object) const
{
	DeepHash hash(object);
	return m_md5 != hash.m_md5;
}

bool DeepHash::operator == (const MD5& md5) const
{
	return m_md5 == md5;
}

bool DeepHash::operator != (const MD5& md5) const
{
	return m_md5 != md5;
}

}
