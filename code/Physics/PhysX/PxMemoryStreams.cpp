#include "Core/Math/MathUtils.h"
#include "Physics/PhysX/PxMemoryStreams.h"

namespace traktor
{
	namespace physics
	{

MemoryOutputStream::MemoryOutputStream()
:	m_data(NULL)
,	m_size(0)
,	m_capacity(0)
{
}

MemoryOutputStream::~MemoryOutputStream()
{
	delete[] m_data;
}

physx::PxU32 MemoryOutputStream::write(const void* src, physx::PxU32 size)
{
	physx::PxU32 expectedSize = m_size + size;
	if(expectedSize > m_capacity)
	{
		m_capacity = expectedSize + 4096;
		physx::PxU8* newData = new physx::PxU8[m_capacity];
		if(newData)
		{
			std::memcpy(newData, m_data, m_size);
			delete[] m_data;
		}
		m_data = newData;
	}
	std::memcpy(m_data + m_size, src, size);
	m_size += size;
	return size;
}

MemoryInputData::MemoryInputData(physx::PxU8* data, physx::PxU32 length)
:	m_size(length)
,	m_data(data)
,	m_pos(0)
{
}

physx::PxU32 MemoryInputData::read(void* dest, physx::PxU32 count)
{
	physx::PxU32 length = min< physx::PxU32 >(count, m_size - m_pos);
	std::memcpy(dest, m_data + m_pos, length);
	m_pos += length;
	return length;
}

physx::PxU32 MemoryInputData::getLength() const
{
	return m_size;
}

void MemoryInputData::seek(physx::PxU32 offset)
{
	m_pos = min< physx::PxU32 >(m_size, offset);
}

physx::PxU32 MemoryInputData::tell() const
{
	return m_pos;
}

	}
}
