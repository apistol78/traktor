#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Misc/Adler32.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.DeepHash", DeepHash, Object)

DeepHash::DeepHash(const Serializable* object)
:	m_hash(0)
{
	if (object)
	{
		Ref< const Serializable > objectRef(object);
		std::vector< uint8_t > copy;

		// Create flat binary copy of object.
		DynamicMemoryStream stream(copy, false, true);
		BinarySerializer(&stream).writeObject(objectRef);

		// Calculate checksum of binary object.
		Adler32 cs;
		cs.begin();
		cs.feed(&copy[0], uint32_t(copy.size()));
		cs.end();

		m_hash = cs.get();
	}
}

uint32_t DeepHash::get() const
{
	return m_hash;
}

bool DeepHash::operator == (const DeepHash& hash) const
{
	return m_hash == hash.m_hash;
}

bool DeepHash::operator != (const DeepHash& hash) const
{
	return m_hash != hash.m_hash;
}

bool DeepHash::operator == (const DeepHash* hash) const
{
	return m_hash == hash->m_hash;
}

bool DeepHash::operator != (const DeepHash* hash) const
{
	return m_hash != hash->m_hash;
}

bool DeepHash::operator == (const Serializable* object) const
{
	DeepHash hash(object);
	return m_hash == hash.m_hash;
}

bool DeepHash::operator != (const Serializable* object) const
{
	DeepHash hash(object);
	return m_hash != hash.m_hash;
}

bool DeepHash::operator == (uint32_t hash) const
{
	return m_hash == hash;
}

bool DeepHash::operator != (uint32_t hash) const
{
	return m_hash != hash;
}

}
