/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#if defined(_PS3)
#	include <alloca.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <sstream>
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/Endian.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{
	namespace
	{

#if defined(T_LITTLE_ENDIAN)

template < typename T >
bool read_primitive(const Ref< IStream >& stream, T& value)
{
	return stream->read(&value, sizeof(T)) == sizeof(T);
}

template < typename T >
bool write_primitive(const Ref< IStream >& stream, T v)
{
	return stream->write(&v, sizeof(T)) == sizeof(T);
}

template < typename T >
bool read_primitives(const Ref< IStream >& stream, T* value, int count)
{
	return stream->read(value, sizeof(T) * count) == sizeof(T) * count;
}

template < typename T >
bool write_primitives(const Ref< IStream >& stream, const T* value, int count)
{
	return stream->write(value, sizeof(T) * count) == sizeof(T) * count;
}

inline bool read_block(const Ref< IStream >& stream, void* block, int count, int size)
{
	return stream->read(block, count * size) == count * size;
}

inline bool write_block(const Ref< IStream >& stream, const void* block, int count, int size)
{
	return stream->write(block, count * size) == count * size;
}

#elif defined(T_BIG_ENDIAN)

template < typename T, int Size >
struct ReadPrimitive
{
	static bool read(IStream* stream, T& t)
	{
		if (stream->read(&t, sizeof(t)) == sizeof(t))
		{
			swap8in64(t);
			return true;
		}
		else
			return false;
	}
};

template < typename T >
struct ReadPrimitive < T, 1 >
{
	static bool read(IStream* stream, T& t)
	{
		return bool(stream->read(&t, 1) == 1);
	}
};

template < typename T >
bool read_primitive(const Ref< IStream >& stream, T& value)
{
	return ReadPrimitive< T, sizeof(T) >::read(stream, value);
}

template < typename T >
bool write_primitive(const Ref< IStream >& stream, T v)
{
	switch (sizeof(T))
	{
	case 1:
		if (stream->write(&v, sizeof(v)) == sizeof(v))
			return true;
	default:
		std::vector< uint8_t > tmp(sizeof(T));
		std::memcpy(&tmp.front(), &v, sizeof(T));
		std::reverse(tmp.begin(), tmp.end());
		if (stream->write(&tmp.front(), sizeof(T)) == sizeof(T))
			return true;
	}
	return false;
}

template < typename T >
bool read_primitives(const Ref< IStream >& stream, T* value, int count)
{
	for (int i = 0; i < count; ++i)
	{
		if (!read_primitive< T >(stream, value[i]))
			return false;
	}
	return true;
}

template < typename T >
bool write_primitives(const Ref< IStream >& stream, const T* value, int count)
{
	for (int i = 0; i < count; ++i)
	{
		if (!write_primitive< T >(stream, value[i]))
			return false;
	}
	return true;
}

bool read_block(const Ref< IStream >& stream, void* block, int count, int size)
{
	int result = stream->read(block, count * size);
	if (result > 0 && size > 1)
	{
		uint8_t* p = static_cast< uint8_t* >(block);
		for (int i = 0; i < result; i += size)
		{
			for (int j = 0; j < size >> 1; ++j)
				std::swap(p[j], p[size - j - 1]);
			p += size;
		}
	}
	return result == count * size;
}

bool write_block(const Ref< IStream >& stream, const void* block, int count, int size)
{
	if (size > 1)
	{
		const uint8_t* p = static_cast< const uint8_t* >(block);
		std::vector< uint8_t > tmp(size);

		for (int i = 0; i < count; ++i, p += size)
		{
			std::memcpy(&tmp.front(), p, size);
			std::reverse(tmp.begin(), tmp.end());
			if (stream->write(&tmp.front(), size) != size)
				return false;
		}

		return true;
	}
	else
	{
		return stream->write(block, count * size) == count * size;
	}
}

#endif

template < >
bool read_primitive(const Ref< IStream >& stream, bool& value)
{
	uint8_t tmp;
	if (stream->read(&tmp, 1) != 1)
		return false;

	value = bool(tmp != 0);
	return true;
}

template < >
bool write_primitive(const Ref< IStream >& stream, bool v)
{
	uint8_t tmp = v ? 0xff : 0x00;
	if (stream->write(&tmp, 1) == 1)
		return true;
	return false;
}

bool read_string(const Ref< IStream >& stream, uint32_t u8len, std::wstring& outString)
{
	if (u8len > 0)
	{
		outString.clear();
		outString.reserve(u8len);

		AutoArrayPtr< uint8_t > buf(new uint8_t [u8len * sizeof(uint8_t)]);
		if (!buf.ptr())
			return false;

		uint8_t* u8str = buf.ptr();
		if (!read_block(stream, u8str, u8len, sizeof(uint8_t)))
			return false;

		Utf8Encoding utf8enc;
		for (uint32_t i = 0; i < u8len; )
		{
			wchar_t ch;
			int n = utf8enc.translate(u8str + i, u8len - i, ch);
			if (n <= 0)
				return false;
			outString += ch;
			i += n;
		}
	}
	else
		outString.clear();

	return true;
}

bool read_string(const Ref< IStream >& stream, std::wstring& outString)
{
    uint32_t u8len = 0;

	if (!read_primitive< uint32_t >(stream, u8len))
		return false;

	return read_string(stream, u8len, outString);
}

bool write_string(const Ref< IStream >& stream, const std::wstring& str)
{
	T_ASSERT (str.length() <= std::numeric_limits< uint16_t >::max());
	
	uint32_t length = uint32_t(str.length());
	if (length > 0)
	{
		uint8_t* buf = (uint8_t*)alloca(length * 6);
		if (!buf)
			return false;

		std::memset(buf, 0, length * 6);

		uint8_t* u8str = (uint8_t*)buf;
		uint32_t u8len;
		
		Utf8Encoding utf8enc;
		u8len = utf8enc.translate(str.c_str(), length, u8str);
		T_FATAL_ASSERT (u8len <= length * 6);

		if (!write_primitive< uint32_t >(stream, u8len))
			return false;

		return write_block(stream, u8str, u8len, sizeof(uint8_t));
	}
	else
	{
		if (!write_primitive< uint32_t >(stream, 0))
			return false;

		return true;
	}
}

bool read_string(const Ref< IStream >& stream, uint32_t u8len, std::string& outString)
{
	outString.clear();
	if (u8len > 0)
	{
		outString.reserve(u8len);
		for (uint32_t i = 0; i < u8len; ++i)
		{
			uint8_t ch;
			if (!read_primitive< uint8_t >(stream, ch))
				return false;
			outString += ch;
		}
	}
	else
		outString.clear();
	return true;
}

bool read_string(const Ref< IStream >& stream, std::string& outString)
{
	uint32_t u8len;

	if (!read_primitive< uint32_t >(stream, u8len))
		return false;

	return read_string(stream, u8len, outString);
}

bool write_string(const Ref< IStream >& stream, const std::string& str)
{
	T_ASSERT (str.length() <= std::numeric_limits< uint16_t >::max());
	
	uint32_t length = uint32_t(str.length());
	if (length > 0)
	{
		if (!write_primitive< uint32_t >(stream, length))
			return false;

		return write_block(stream, str.c_str(), length, sizeof(uint8_t));
	}
	else
	{
		if (!write_primitive< uint32_t >(stream, 0))
			return false;

		return true;
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.BinarySerializer", BinarySerializer, Serializer);

// Disable "Unusual use of Boolean expression"
/*lint -e514*/

#define T_CHECK_STATUS \
	if (failed()) return;

BinarySerializer::BinarySerializer(IStream* stream)
:	m_stream(stream)
,	m_direction(m_stream->canRead() ? SdRead : SdWrite)
,	m_nextCacheId(1)
,	m_nextTypeCacheId(0)
{
}

Serializer::Direction BinarySerializer::getDirection() const
{
	return m_direction;
}

void BinarySerializer::operator >> (const Member< bool >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_primitive< bool >(m_stream, m);
	else
		write_primitive< bool >(m_stream, m);
}

void BinarySerializer::operator >> (const Member< int8_t >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_primitive< int8_t >(m_stream, m);
	else
		write_primitive< int8_t >(m_stream, m);
}

void BinarySerializer::operator >> (const Member< uint8_t >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_primitive< uint8_t >(m_stream, m);
	else
		write_primitive< uint8_t >(m_stream, m);
}

void BinarySerializer::operator >> (const Member< int16_t >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_primitive< int16_t >(m_stream, m);
	else
		write_primitive< int16_t >(m_stream, m);
}

void BinarySerializer::operator >> (const Member< uint16_t >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_primitive< uint16_t >(m_stream, m);
	else
		write_primitive< uint16_t >(m_stream, m);
}

void BinarySerializer::operator >> (const Member< int32_t >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_primitive< int32_t >(m_stream, m);
	else
		write_primitive< int32_t >(m_stream, m);
}

void BinarySerializer::operator >> (const Member< uint32_t >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_primitive< uint32_t >(m_stream, m);
	else
		write_primitive< uint32_t >(m_stream, m);
}

void BinarySerializer::operator >> (const Member< int64_t >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_primitive< int64_t >(m_stream, m);
	else
		write_primitive< int64_t >(m_stream, m);
}

void BinarySerializer::operator >> (const Member< uint64_t >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_primitive< uint64_t >(m_stream, m);
	else
		write_primitive< uint64_t >(m_stream, m);
}

void BinarySerializer::operator >> (const Member< float >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_primitive< float >(m_stream, m);
	else
		write_primitive< float >(m_stream, m);
}

void BinarySerializer::operator >> (const Member< double >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_primitive< double >(m_stream, m);
	else
		write_primitive< double >(m_stream, m);
}

void BinarySerializer::operator >> (const Member< std::string >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_string(m_stream, m);
	else
		write_string(m_stream, m);
}

void BinarySerializer::operator >> (const Member< std::wstring >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_string(m_stream, m);
	else
		write_string(m_stream, m);
}

void BinarySerializer::operator >> (const Member< Guid >& m)
{
	T_CHECK_STATUS;

	Guid& guid = m;
	if (m_direction == SdRead)
	{
		bool validGuid = false;
		
		if (!read_primitive< bool >(m_stream, validGuid))
			return;

		if (validGuid)
		{
			uint8_t data[16];
			read_block(m_stream, data, 16, 1);
			guid = Guid(data);
		}
		else
			guid = Guid();
	}
	else
	{
		bool validGuid = guid.isValid();
		if (!write_primitive< bool >(m_stream, validGuid))
			return;

		if (validGuid)
			write_block(m_stream, static_cast< const uint8_t* >(guid), 16, 1);
	}
}

void BinarySerializer::operator >> (const Member< Path >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
	{
		std::wstring path;
		read_string(m_stream, path);
		*m = Path(path);
	}
	else
	{
		std::wstring path = m->getOriginal();
		write_string(m_stream, path);
	}
}

void BinarySerializer::operator >> (const Member< Color4ub >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
	{
		read_primitive< uint8_t >(m_stream, m->r);
		read_primitive< uint8_t >(m_stream, m->g);
		read_primitive< uint8_t >(m_stream, m->b);
		read_primitive< uint8_t >(m_stream, m->a);
	}
	else
	{
		write_primitive< uint8_t >(m_stream, m->r);
		write_primitive< uint8_t >(m_stream, m->g);
		write_primitive< uint8_t >(m_stream, m->b);
		write_primitive< uint8_t >(m_stream, m->a);
	}
}

void BinarySerializer::operator >> (const Member< Color4f >& m)
{
	T_CHECK_STATUS;

	float T_MATH_ALIGN16 e[4];
	if (m_direction == SdRead)
	{
		read_primitives< float >(m_stream, e, 4);
		(*m) = Color4f::loadUnaligned(e);
	}
	else
	{
		(*m).storeUnaligned(e);
		write_primitives< float >(m_stream, e, 4);
	}
}

void BinarySerializer::operator >> (const Member< Scalar >& m)
{
	T_CHECK_STATUS;

	Scalar& v = m;
	if (m_direction == SdRead)
	{
		float tmp;
		read_primitive< float >(m_stream, tmp);
		v = Scalar(tmp);
	}
	else
		write_primitive< float >(m_stream, float(v));
}

void BinarySerializer::operator >> (const Member< Vector2 >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
	{
		read_primitive< float >(m_stream, m->x);
		read_primitive< float >(m_stream, m->y);
	}
	else
	{
		write_primitive< float >(m_stream, m->x);
		write_primitive< float >(m_stream, m->y);
	}
}

void BinarySerializer::operator >> (const Member< Vector4 >& m)
{
	T_CHECK_STATUS;

	float T_MATH_ALIGN16 e[4];
	if (m_direction == SdRead)
	{
		read_primitives< float >(m_stream, e, 4);
		(*m) = Vector4::loadUnaligned(e);
	}
	else
	{
		(*m).storeUnaligned(e);
		write_primitives< float >(m_stream, e, 4);
	}
}

void BinarySerializer::operator >> (const Member< Matrix33 >& m)
{
	T_CHECK_STATUS;

	if (m_direction == SdRead)
	{
		for (int i = 0; i < 3 * 3; ++i)
			read_primitive< float >(m_stream, m->m[i]);
	}
	else
	{
		for (int i = 0; i < 3 * 3; ++i)
			write_primitive< float >(m_stream, m->m[i]);
	}
}

void BinarySerializer::operator >> (const Member< Matrix44 >& m)
{
	T_CHECK_STATUS;

	float T_MATH_ALIGN16 values[16];
	if (m_direction == SdRead)
	{
		read_primitives< float >(m_stream, values, 16);
		(*m) = Matrix44::loadUnaligned(values);
	}
	else
	{
		(*m).storeUnaligned(values);
		write_primitives< float >(m_stream, values, 16);
	}
}

void BinarySerializer::operator >> (const Member< Quaternion >& m)
{
	T_CHECK_STATUS;

	float T_MATH_ALIGN16 e[4];
	if (m_direction == SdRead)
	{
		read_primitives< float >(m_stream, e, 4);
		m->e = Vector4::loadUnaligned(e);
	}
	else
	{
		m->e.storeUnaligned(e);
		write_primitives< float >(m_stream, e, 4);
	}
}

void BinarySerializer::operator >> (const Member< ISerializable* >& m)
{
	T_CHECK_STATUS;

	if (m_direction == SdRead)
	{
		bool reference = false;
		uint64_t hash = 0;

		if (!ensure(read_primitive< bool >(m_stream, reference)))
			return;
		if (!ensure(read_primitive< uint64_t >(m_stream, hash)))
			return;

		Ref< ISerializable > object;

		if (hash)
		{
			if (!reference)
			{
				uint32_t typeHashOrLen;
				const TypeInfo* type;
				int32_t version;

				if (!ensure(read_primitive< uint32_t >(m_stream, typeHashOrLen)))
				{
					log::error << L"Unable to serialize \"" << m.getName() << L"\"; unable to read type hash" << Endl;
					return;
				}

				if ((typeHashOrLen & 0x80000000) == 0x80000000)
					type = m_typeReadCache[typeHashOrLen & 0x7fffffff];
				else
				{
					std::wstring typeName;
					if (!ensure(read_string(m_stream, typeHashOrLen, typeName)))
					{
						log::error << L"Unable to serialize \"" << m.getName() << L"\"; unable to read type" << Endl;
						return;
					}

					type = TypeInfo::find(typeName);
					if (!ensure(type != 0))
					{
						log::error << L"Unable to serialize \"" << m.getName() << L"\"; no such type \"" << typeName << L"\"" << Endl;
						return;
					}

					m_typeReadCache.push_back(type);
				}

				T_ASSERT (type);
				
				object = checked_type_cast< ISerializable* >(type->createInstance());
				if (!ensure(object != 0))
				{
					log::error << L"Unable to serialize \"" << m.getName() << L"\"; unable to create type \"" << type->getName() << L"\"" << Endl;
					return;
				}

				if (!ensure(read_primitive< int32_t >(m_stream, version)))
				{
					log::error << L"Unable to serialize \"" << m.getName() << L"\"; unable to read version" << Endl;
					return;
				}

				serialize(object, version);

				m_readCache[hash] = object;
			}
			else
			{
				object = static_cast< ISerializable* >(m_readCache[hash]);
				if (!ensure(object != 0))
				{
					log::error << L"Unable to serialize \"" << m.getName() << L"\"; no such reference" << Endl;
					return;
				}
			}
		}

		m = object;
	}
	else
	{
		Ref< ISerializable > object = *m;
		if (object)
		{
			T_ASSERT (type_of(object).isInstantiable());

			std::map< ISerializable*, uint64_t >::iterator i = m_writeCache.find(object);
			if (i != m_writeCache.end())
			{
				if (!write_primitive< bool >(m_stream, true))
					return;
				if (!write_primitive< uint64_t >(m_stream, i->second))
					return;
			}
			else
			{
				int32_t version = type_of(object).getVersion();
				uint64_t hash = m_nextCacheId++;

				if (!write_primitive< bool >(m_stream, false))
					return;
				if (!write_primitive< uint64_t >(m_stream, hash))
					return;

				std::map< const TypeInfo*, uint32_t >::const_iterator it = m_typeWriteCache.find(&type_of(object));
				if (it != m_typeWriteCache.end())
					write_primitive< uint32_t >(m_stream, 0x80000000 | it->second);
				else
				{
					if (!write_string(m_stream, type_name(object)))
						return;

					m_typeWriteCache.insert(std::make_pair(&type_of(object), m_nextTypeCacheId++));
				}

				if (!write_primitive< int32_t >(m_stream, version))
					return;

				m_writeCache[object] = hash;
				serialize(object, version);
			}
		}
		else
		{
			write_primitive< bool >(m_stream, false);
			write_primitive< uint64_t >(m_stream, 0ULL);
		}
	}
}

void BinarySerializer::operator >> (const Member< void* >& m)
{
	T_CHECK_STATUS;

	if (m_direction == SdRead)
	{
		uint32_t size;
			
		if (!ensure(read_primitive< uint32_t >(m_stream, size)))
			return;
		
		if (!ensure(size <= m.getBlobSize()))
			return;

		m.setBlobSize(size);

		if (size > 0)
			read_block(m_stream, m.getBlob(), size, 1);
	}
	else
	{
		uint32_t size = m.getBlobSize();

		if (!ensure(write_primitive< uint32_t >(m_stream, size)))
			return;

		if (size > 0)
			write_block(m_stream, m.getBlob(), size, 1);
	}
}

void BinarySerializer::operator >> (const MemberArray& m)
{
	T_CHECK_STATUS;

	if (m_direction == SdRead)
	{
		uint32_t size;

		if (!ensure(read_primitive< uint32_t >(m_stream, size)))
			return;
		
		m.reserve(size, size);
		for (uint32_t i = 0; i < size; ++i)
		{
			T_CHECK_STATUS;
			m.read(*this);
		}
	}
	else
	{
		uint32_t size = uint32_t(m.size());

		if (!ensure(write_primitive< uint32_t >(m_stream, size)))
			return;

		for (uint32_t i = 0; i < size; ++i)
		{
			T_CHECK_STATUS;
			m.write(*this);
		}
	}
}

void BinarySerializer::operator >> (const MemberComplex& m)
{
	T_CHECK_STATUS;
	m.serialize(*this);
}

void BinarySerializer::operator >> (const MemberEnumBase& m)
{
	T_CHECK_STATUS;
	m.serialize(*this);
}

/*lint -restore*/

}
