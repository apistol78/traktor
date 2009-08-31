#include <sstream>
#if defined(T_BIG_ENDIAN)
#	include <algorithm>
#endif
#include <limits>
#include <algorithm>
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Io/Stream.h"

namespace traktor
{
	namespace
	{

#if defined(T_LITTLE_ENDIAN)

template < typename T > bool read_primitive(const Ref< Stream >& stream, T& value)
{
	return stream->read(&value, sizeof(T)) == sizeof(T);
}

template < typename T > bool write_primitive(const Ref< Stream >& stream, T v)
{
	return stream->write(&v, sizeof(T)) == sizeof(T);
}

inline bool read_block(const Ref< Stream >& stream, void* block, int count, int size)
{
	return stream->read(block, count * size) == count * size;
}

inline bool write_block(const Ref< Stream >& stream, const void* block, int count, int size)
{
	return stream->write(block, count * size) == count * size;
}

#elif defined(T_BIG_ENDIAN)

template < typename T > bool read_primitive(const Ref< Stream >& stream, T& value)
{
	switch (sizeof(T))
	{
	case 1:
		{
			if (stream->read(&value, sizeof(T)) == sizeof(T))
				return true;
		}
	default:
		{
			std::vector< uint8_t > tmp(sizeof(T));
			if (stream->read(&tmp[0], sizeof(T)) == sizeof(T))
			{
				std::reverse(tmp.begin(), tmp.end());
				value = *(T*)&tmp[0];
				return true;
			}
			break;
		}
	}
	return false;
}

template < typename T > bool write_primitive(const Ref< Stream >& stream, T v)
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

bool read_block(const Ref< Stream >& stream, void* block, int count, int size)
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

bool write_block(const Ref< Stream >& stream, const void* block, int count, int size)
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

bool read_primitive(const Ref< Stream >& stream, bool& value)
{
	uint8_t tmp;
	if (stream->read(&tmp, 1) != 1)
		return false;

	value = bool(tmp != 0);
	return true;
}

bool write_primitive(const Ref< Stream >& stream, bool v)
{
	uint8_t tmp = v ? 0xff : 0x00;
	if (stream->write(&tmp, 1) == 1)
		return true;
	return false;
}

template < typename StringType >
bool read_string(const Ref< Stream >& stream, StringType& outString)
{
	uint16_t length;
	
	if (!read_primitive< uint16_t >(stream, length))
		return false;

	if (length > 0)
	{
		std::vector< typename StringType::value_type > tmp(length);
		if (!read_block(stream, &tmp[0], length, sizeof(typename StringType::value_type)))
			return false;

		outString = StringType(tmp.begin(), tmp.end());
	}
	else
		outString.clear();

	return true;
}

template < typename StringType >
bool write_string(const Ref< Stream >& stream, const StringType& str)
{
	T_ASSERT (str.length() <= std::numeric_limits< uint16_t >::max());
	
	uint16_t length = uint16_t(str.length());
	if (!write_primitive< uint16_t >(stream, length))
		return false;

	if (length > 0)
		return write_block(stream, str.c_str(), length, sizeof(typename StringType::value_type));
	else
		return true;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.BinarySerializer", BinarySerializer, Serializer)

// Disable "Unusual use of Boolean expression"
/*lint -e514*/

BinarySerializer::BinarySerializer(Stream* stream)
:	m_stream(stream)
,	m_direction(m_stream->canRead() ? SdRead : SdWrite)
,	m_nextCacheId(1)
{
}

Serializer::Direction BinarySerializer::getDirection()
{
	return m_direction;
}

bool BinarySerializer::operator >> (const Member< bool >& m)
{
	if (m_direction == SdRead)
		return read_primitive< bool >(m_stream, m);
	else
		return write_primitive< bool >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< int8_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< int8_t >(m_stream, m);
	else
		return write_primitive< int8_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< uint8_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< uint8_t >(m_stream, m);
	else
		return write_primitive< uint8_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< int16_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< int16_t >(m_stream, m);
	else
		return write_primitive< int16_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< uint16_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< uint16_t >(m_stream, m);
	else
		return write_primitive< uint16_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< int32_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< int32_t >(m_stream, m);
	else
		return write_primitive< int32_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< uint32_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< uint32_t >(m_stream, m);
	else
		return write_primitive< uint32_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< int64_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< int64_t >(m_stream, m);
	else
		return write_primitive< int64_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< uint64_t >& m)
{
	if (m_direction == SdRead)
		return read_primitive< uint64_t >(m_stream, m);
	else
		return write_primitive< uint64_t >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< float >& m)
{
	if (m_direction == SdRead)
		return read_primitive< float >(m_stream, m);
	else
		return write_primitive< float >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< double >& m)
{
	if (m_direction == SdRead)
		return read_primitive< double >(m_stream, m);
	else
		return write_primitive< double >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< std::string >& m)
{
	if (m_direction == SdRead)
		return read_string< std::string >(m_stream, m);
	else
		return write_string< std::string >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< std::wstring >& m)
{
	if (m_direction == SdRead)
		return read_string< std::wstring >(m_stream, m);
	else
		return write_string< std::wstring >(m_stream, m);
}

bool BinarySerializer::operator >> (const Member< Guid >& m)
{
	Guid& guid = m;
	if (m_direction == SdRead)
	{
		bool validGuid;
		
		if (!read_primitive< bool >(m_stream, validGuid))
			return false;

		if (validGuid)
		{
			uint8_t data[16];
			if (!read_block(m_stream, data, 16, 1))
				return false;
			guid = Guid(data);
		}
		else
			guid = Guid();
	}
	else
	{
		bool validGuid = guid.isValid();
		if (!write_primitive< bool >(m_stream, validGuid))
			return false;

		if (validGuid)
		{
			if (!write_block(m_stream, static_cast< const uint8_t* >(guid), 16, 1))
				return false;
		}
	}
	return true;
}

bool BinarySerializer::operator >> (const Member< Path >& m)
{
	if (m_direction == SdRead)
	{
		std::wstring path;
		if (!read_string< std::wstring >(m_stream, path))
			return false;
		*m = Path(path);
	}
	else
	{
		std::wstring path = m->getPathName();
		if (!write_string(m_stream, path))
			return false;
	}
	return true;
}

bool BinarySerializer::operator >> (const Member< Color >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		result &= read_primitive< uint8_t >(m_stream, m->r);
		result &= read_primitive< uint8_t >(m_stream, m->g);
		result &= read_primitive< uint8_t >(m_stream, m->b);
		result &= read_primitive< uint8_t >(m_stream, m->a);
	}
	else
	{
		result &= write_primitive< uint8_t >(m_stream, m->r);
		result &= write_primitive< uint8_t >(m_stream, m->g);
		result &= write_primitive< uint8_t >(m_stream, m->b);
		result &= write_primitive< uint8_t >(m_stream, m->a);
	}
	return result;
}

bool BinarySerializer::operator >> (const Member< Scalar >& m)
{
	Scalar& v = m;
	if (m_direction == SdRead)
	{
		float tmp;
		if (!read_primitive< float >(m_stream, tmp))
			return false;
		v = Scalar(tmp);
	}
	else
	{
		if (!write_primitive< float >(m_stream, float(v)))
			return false;
	}
	return true;
}

bool BinarySerializer::operator >> (const Member< Vector2 >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		result &= read_primitive< float >(m_stream, m->x);
		result &= read_primitive< float >(m_stream, m->y);
	}
	else
	{
		result &= write_primitive< float >(m_stream, m->x);
		result &= write_primitive< float >(m_stream, m->y);
	}
	return result;
}

bool BinarySerializer::operator >> (const Member< Vector4 >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		float x, y, z, w;
		result &= read_primitive< float >(m_stream, x);
		result &= read_primitive< float >(m_stream, y);
		result &= read_primitive< float >(m_stream, z);
		result &= read_primitive< float >(m_stream, w);
		if (result)
			m->set(x, y, z, w);
	}
	else
	{
		result &= write_primitive< float >(m_stream, m->x());
		result &= write_primitive< float >(m_stream, m->y());
		result &= write_primitive< float >(m_stream, m->z());
		result &= write_primitive< float >(m_stream, m->w());
	}
	return result;
}

bool BinarySerializer::operator >> (const Member< Matrix33 >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		for (int i = 0; i < 3 * 3; ++i)
			result &= read_primitive< float >(m_stream, m->m[i]);
	}
	else
	{
		for (int i = 0; i < 3 * 3; ++i)
			result &= write_primitive< float >(m_stream, m->m[i]);
	}
	return result;
}

bool BinarySerializer::operator >> (const Member< Matrix44 >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		for (int r = 0; r < 4; ++r)
		{
			for (int c = 0; c < 4; ++c)
			{
				float value;
				result &= read_primitive< float >(m_stream, value);
				(*m).set(r, c, Scalar(value));
			}
		}
	}
	else
	{
		for (int r = 0; r < 4; ++r)
		{
			for (int c = 0; c < 4; ++c)
				result &= write_primitive< float >(m_stream, (*m).get(r, c));
		}
	}
	return result;
}

bool BinarySerializer::operator >> (const Member< Quaternion >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		for (int i = 0; i < 4; ++i)
			result &= read_primitive< float >(m_stream, m->e[i]);
	}
	else
	{
		for (int i = 0; i < 4; ++i)
			result &= write_primitive< float >(m_stream, m->e[i]);
	}
	return result;
}

bool BinarySerializer::operator >> (const Member< Serializable >& m)
{
	Serializable* o = &static_cast< Serializable& >(m);
	if (m_direction == SdRead)
	{
		int32_t version;
		if (!read_primitive< int32_t >(m_stream, version))
			return false;
		return serialize(o, version);
	}
	else
	{
		int32_t version = o->getVersion();
		if (!write_primitive< int32_t >(m_stream, version))
			return false;
		return serialize(o, version);
	}
}

bool BinarySerializer::operator >> (const Member< Serializable* >& m)
{
	if (m_direction == SdRead)
	{
		bool reference;
		uint64_t hash;

		if (!read_primitive< bool >(m_stream, reference))
			return false;
		if (!read_primitive< uint64_t >(m_stream, hash))
			return false;

		Ref< Serializable > o;

		if (hash)
		{
			if (!reference)
			{
				std::wstring typeName;
				int32_t version;

				if (!read_string< std::wstring >(m_stream, typeName))
					return false;
				
				const Type* type = Type::find(typeName);
				if (type == 0)
					return false;
					
				if (!(o = checked_type_cast< Serializable* >(type->newInstance())))
					return false;

				if (!read_primitive< int32_t >(m_stream, version))
					return false;

				if (!serialize(o, version))
					return false;

				m_readCache[hash] = o;
			}
			else
			{
				o = static_cast< Serializable* >(m_readCache[hash]);
				if (!o)
					return false;
			}
		}

		m = o;
	}
	else
	{
		Ref< Serializable > o = *m;

		if (o)
		{
			std::map< Ref< Serializable >, uint64_t >::iterator i = m_writeCache.find(o);
			if (i != m_writeCache.end())
			{
				if (!write_primitive< bool >(m_stream, true))
					return false;
				if (!write_primitive< uint64_t >(m_stream, i->second))
					return false;
			}
			else
			{
				int32_t version = o->getVersion();
				uint64_t hash = m_nextCacheId++;

				if (!write_primitive< bool >(m_stream, false))
					return false;
				if (!write_primitive< uint64_t >(m_stream, hash))
					return false;
				if (!write_string< std::wstring >(m_stream, type_name(o)))
					return false;
				if (!write_primitive< int32_t >(m_stream, version))
					return false;

				m_writeCache[o] = hash;
				if (!serialize(o, version))
					return false;
			}
		}
		else
		{
			if (!write_primitive< bool >(m_stream, false))
				return false;
			if (!write_primitive< uint64_t >(m_stream, 0ULL))
				return false;
		}
	}
	return true;
}

bool BinarySerializer::operator >> (const Member< void* >& m)
{
	if (m_direction == SdRead)
	{
		uint32_t size;
			
		if (!read_primitive< uint32_t >(m_stream, size))
			return false;
		
		if (size > m.getBlobSize())
			return false;

		m.setBlobSize(size);
		return read_block(m_stream, m.getBlob(), size, 1);
	}
	else
	{
		if (!write_primitive< uint32_t >(m_stream, uint32_t(m.getBlobSize())))
			return false;

		return write_block(m_stream, m.getBlob(), m.getBlobSize(), 1);
	}
}

bool BinarySerializer::operator >> (const MemberArray& m)
{
	if (m_direction == SdRead)
	{
		uint32_t size;

		if (!read_primitive< uint32_t >(m_stream, size))
			return false;
		
		m.reserve(size);
		for (uint32_t i = 0; i < size; ++i)
		{
			if (!m.serialize(*this, i))
				return false;
		}
	}
	else
	{
		uint32_t size = uint32_t(m.size());

		if (!write_primitive< uint32_t >(m_stream, size))
			return false;

		for (uint32_t i = 0; i < size; ++i)
		{
			if (!m.serialize(*this, i))
				return false;
		}
	}
	return true;
}

bool BinarySerializer::operator >> (const MemberComplex& m)
{
	return m.serialize(*this);
}

/*lint -restore*/

}
