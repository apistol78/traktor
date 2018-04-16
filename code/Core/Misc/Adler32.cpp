/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/Adler32.h"

namespace traktor
{
	namespace
	{

const uint32_t c_modAdler = 65521;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Adler32", Adler32, IHash)

Adler32::Adler32()
:	m_A(0)
,	m_B(0)
,	m_feed(0)
{
}

void Adler32::begin()
{
	m_A = 1;
	m_B = 0;
	m_feed = 0;
}

void Adler32::feed(const void* buffer, uint64_t bufferSize)
{
	for (uint32_t i = 0; i < bufferSize; ++i)
	{
		uint8_t byte = static_cast< const uint8_t* >(buffer)[i];

		m_A += byte;
		m_B += m_A;

		if (++m_feed >= 5550)
		{
			m_A = (m_A & 0xffff) + (m_A >> 16) * (65536 - c_modAdler);
			m_B = (m_B & 0xffff) + (m_B >> 16) * (65536 - c_modAdler);
			m_feed = 0;
		}
	}
}

void Adler32::end()
{
	if (m_feed > 0)
	{
		m_A = (m_A & 0xffff) + (m_A >> 16) * (65536 - c_modAdler);
		m_B = (m_B & 0xffff) + (m_B >> 16) * (65536 - c_modAdler);
	}
}

uint32_t Adler32::get() const
{
	return (m_B << 16) | m_A;
}

}
