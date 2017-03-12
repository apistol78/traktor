#include <algorithm>
#include "Core/Io/Writer.h"

namespace traktor
{
	namespace
	{

#if defined(T_LITTLE_ENDIAN)

template <typename T> bool write_primitive(IStream* stream, T v)
{
	if (stream->write(&v, sizeof(T)) == sizeof(T))
		return true;
	return false;
}

#elif defined(T_BIG_ENDIAN)

template <typename T> bool write_primitive(IStream* stream, T v)
{
	std::vector< uint8_t > tmp(sizeof(T));
	std::memcpy(&tmp.front(), &v, sizeof(T));
	std::reverse(tmp.begin(), tmp.end());
	if (stream->write(&tmp.front(), sizeof(T)) == sizeof(T))
		return true;
	return false;
}

#endif

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Writer", Writer, Object)

Writer::Writer(IStream* stream)
:	m_stream(stream)
{
	T_ASSERT (m_stream);
	T_ASSERT (m_stream->canWrite());
}

Writer& Writer::operator << (bool v)
{
	uint8_t tmp = v ? 1 : 0;
	m_stream->write(&tmp, 1);
	return *this;
}

Writer& Writer::operator << (int8_t c)
{
	write_primitive< int8_t >(m_stream, c);
	return *this;
}

Writer& Writer::operator << (uint8_t uc)
{
	write_primitive< uint8_t >(m_stream, uc);
	return *this;
}

Writer& Writer::operator << (int16_t i)
{
	write_primitive< int16_t >(m_stream, i);
	return *this;
}

Writer& Writer::operator << (uint16_t ui)
{
	write_primitive< uint16_t >(m_stream, ui);
	return *this;
}

Writer& Writer::operator << (int32_t i)
{
	write_primitive< int32_t >(m_stream, i);
	return *this;
}

Writer& Writer::operator << (uint32_t ui)
{
	write_primitive< uint32_t >(m_stream, ui);
	return *this;
}

Writer& Writer::operator << (int64_t i)
{
	write_primitive< int64_t >(m_stream, i);
	return *this;
}

Writer& Writer::operator << (uint64_t i)
{
	write_primitive< uint64_t >(m_stream, i);
	return *this;
}

Writer& Writer::operator << (float f)
{
	write_primitive< float >(m_stream, f);
	return *this;
}

Writer& Writer::operator << (double f)
{
	write_primitive< double >(m_stream, f);
	return *this;
}

Writer& Writer::operator << (const std::wstring& s)
{
	size_t length = s.length();
	for (size_t i = 0; i < length; ++i)
	{
		uint16_t ch = uint16_t(s[i]);
		write_primitive< uint16_t >(m_stream, ch);
	}
	write_primitive< uint16_t >(m_stream, 0);
	return *this;
}

Writer& Writer::operator << (const wchar_t* s)
{
	return *this << std::wstring(s);
}

int64_t Writer::write(const void* block, int64_t nbytes)
{
	return m_stream->write(block, nbytes);
}

int64_t Writer::write(const void* block, int64_t count, int64_t size)
{
#if defined(T_BIG_ENDIAN)

	if (size > 1)
	{
		const uint8_t* p = static_cast< const uint8_t* >(block);
		std::vector< uint8_t > tmp(size);
		
		for (int64_t i = 0; i < count; ++i, p += size)
		{
			std::memcpy(&tmp.front(), p, size);
			std::reverse(tmp.begin(), tmp.end());
			if (m_stream->write(&tmp.front(), size) != size)
				return -1;
		}
		
		return count * size;
	}

#endif

	return m_stream->write(block, count * size);
}

}
