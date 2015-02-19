#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Ps3/Blob.h"

namespace traktor
{
	namespace render
	{

Blob::Blob()
:	m_size(0)
,	m_data(0)
{
}

Blob::Blob(const Blob& blob)
:	m_size(blob.m_size)
,	m_data(new uint8_t [blob.m_size])
{
	std::memcpy(m_data, blob.m_data, m_size);
}

Blob::Blob(uint32_t size)
:	m_size(size)
,	m_data(new uint8_t [size])
{
}

Blob::Blob(uint32_t size, const void* data)
:	m_size(size)
,	m_data(new uint8_t [size])
{
	std::memcpy(m_data, data, m_size);
}

Blob::~Blob()
{
	delete[] m_data;
}

void* Blob::getData() const
{
	return m_data;
}

uint32_t Blob::getSize() const
{
	return m_size;
}

bool Blob::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"size", m_size);

	if (s.getDirection() == ISerializer::SdRead)
	{
		delete[] m_data;
		m_data = new uint8_t [m_size];
	}

	s >> Member< void* >(L"data", m_data, m_size);

	return true;
}

Blob& Blob::operator = (const Blob& blob)
{
	delete[] m_data;

	m_size = blob.m_size;
	m_data = new uint8_t [m_size];

	std::memcpy(m_data, blob.m_data, m_size);
	
	return *this;
}

	}
}
