#include <algorithm>
#include <vector>
#include "Core/Io/Reader.h"
#include "Core/Misc/Endian.h"

namespace traktor
{
	namespace
	{

#if defined(T_LITTLE_ENDIAN)

template <typename T> T read_primitive(IStream* stream)
{
	T tmp;
	if (stream->read(&tmp, sizeof(T)) == sizeof(T))
		return tmp;
	return T(0);
}

#elif defined(T_BIG_ENDIAN)

template < typename T, int Size >
struct ReadPrimitive
{
	static T read(IStream* stream)
	{
		T t;
		stream->read(&t, sizeof(t));
		swap8in32(t);
		return t;
	}
};

template < typename T >
struct ReadPrimitive < T, 1 >
{
	static T read(IStream* stream)
	{
		T t;
		stream->read(&t, 1);
		return t;
	}
};

template < typename T >
struct ReadPrimitive < T, 8 >
{
	static T read(IStream* stream)
	{
		T t;
		stream->read(&t, 8);
		swap8in32(*(((uint32_t*)&t) + 0));
		swap8in32(*(((uint32_t*)&t) + 1));
		return t;
	}
};

template <typename T> T read_primitive(IStream* stream)
{
	return ReadPrimitive< T, sizeof(T) >::read(stream);
}

#endif

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Reader", Reader, Object)

Reader::Reader(IStream* stream)
:	m_stream(stream)
{
	T_ASSERT (m_stream);
	T_ASSERT (m_stream->canRead());
}

Reader& Reader::operator >> (bool& b)
{
	uint8_t tmp = 0;
	m_stream->read(&tmp, 1);
	b = tmp ? true : false;
	return *this;
}

Reader& Reader::operator >> (int8_t& c)
{
	c = read_primitive< int8_t >(m_stream);
	return *this;
}

Reader& Reader::operator >> (uint8_t& uc)
{
	uc = read_primitive< uint8_t >(m_stream);
	return *this;
}

Reader& Reader::operator >> (int16_t& i)
{
	i = read_primitive< int16_t >(m_stream);
	return *this;
}

Reader& Reader::operator >> (uint16_t& ui)
{
	ui = read_primitive< uint16_t >(m_stream);
	return *this;
}

Reader& Reader::operator >> (int32_t& i)
{
	i = read_primitive< int32_t >(m_stream);
	return *this;
}

Reader& Reader::operator >> (uint32_t& ui)
{
	ui = read_primitive< uint32_t >(m_stream);
	return *this;
}

Reader& Reader::operator >> (int64_t& i)
{
	i = read_primitive< int64_t >(m_stream);
	return *this;
}

Reader& Reader::operator >> (uint64_t& ui)
{
	ui = read_primitive< uint64_t >(m_stream);
	return *this;
}

Reader& Reader::operator >> (float& f)
{
	f = read_primitive< float >(m_stream);
	return *this;
}

Reader& Reader::operator >> (double& f)
{
	f = read_primitive< double >(m_stream);
	return *this;
}

Reader& Reader::operator >> (std::wstring& s)
{
	s.clear();
	for (;;)
	{
		uint16_t ch;
		if (read(&ch, 1, sizeof(ch)) != sizeof(ch) || ch == '\0')
			break;
		s += wchar_t(ch);
	}
	return *this;
}

int64_t Reader::read(void* block, int64_t nbytes)
{
	return m_stream->read(block, nbytes);
}

int64_t Reader::read(void* block, int64_t count, int64_t size)
{
	int64_t result;
	
	result = m_stream->read(block, count * size);
	
#if defined(T_BIG_ENDIAN)

	if (result > 0 && size > 1)
	{
		char* p = static_cast< char* >(block);
		for (int64_t i = 0; i < result; i += size)
		{
			for (int64_t j = 0; j < size >> 1; ++j)
				std::swap(p[j], p[size - j - 1]);
			p += size;
		}
	}

#endif
		
	return result;
}

int64_t Reader::skip(int64_t nbytes)
{
	return m_stream->seek(IStream::SeekCurrent, nbytes);
}

}
