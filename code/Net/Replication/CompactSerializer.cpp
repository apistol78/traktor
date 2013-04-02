#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeRange.h"
#include "Net/Replication/CompactSerializer.h"
#include "Net/Replication/Pack.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

bool read_bool(BitReader& r, bool& value)
{
	value = r.readBit();
	return true;
}

bool read_int8(BitReader& r, int8_t& value)
{
	value = r.readSigned(8);
	return true;
}

bool read_uint8(BitReader& r, uint8_t& value)
{
	value = r.readUnsigned(8);
	return true;
}

bool read_int16(BitReader& r, int16_t& value)
{
	bool sign = r.readBit();
	int32_t uv = 0;

	if (r.readBit())
		uv = r.readUnsigned(15);
	else
		uv = r.readUnsigned(8);

	value = sign ? -uv : uv;
	return true;
}

bool read_uint16(BitReader& r, uint16_t& value)
{
	if (r.readBit())
		value = r.readUnsigned(16);
	else
		value = r.readUnsigned(8);
	return true;
}

bool read_int32(BitReader& r, int32_t& value)
{
	bool sign = r.readBit();
	int32_t uv = 0;

	if (r.readBit())
		uv = r.readUnsigned(31);
	else
		uv = r.readUnsigned(16);

	value = sign ? -uv : uv;
	return true;
}

bool read_uint32(BitReader& r, uint32_t& value)
{
	if (r.readBit())
		value = r.readUnsigned(32);
	else
		value = r.readUnsigned(16);
	return true;
}

bool read_float(BitReader& r, float& value)
{
	uint32_t v = r.readUnsigned(32);
	value = *(float*)&v;
	return true;
}

bool write_bool(BitWriter& w, bool v)
{
	w.writeBit(v);
	return true;
}

bool write_int8(BitWriter& w, int8_t v)
{
	w.writeSigned(8, v);
	return true;
}

bool write_uint8(BitWriter& w, uint8_t v)
{
	w.writeUnsigned(8, v);
	return true;
}

bool write_int16(BitWriter& w, int16_t v)
{
	if (v > 0)
		w.writeBit(false);
	else
	{
		v = -v;
		w.writeBit(true);
	}

	if (v > 255)
	{
		w.writeBit(true);
		w.writeUnsigned(15, v);
	}
	else
	{
		w.writeBit(false);
		w.writeUnsigned(8, v);
	}

	return true;
}

bool write_uint16(BitWriter& w, uint16_t v)
{
	if (v > 255)
	{
		w.writeBit(true);
		w.writeUnsigned(16, v);
	}
	else
	{
		w.writeBit(false);
		w.writeUnsigned(8, v);
	}
	return true;
}

bool write_int32(BitWriter& w, int32_t v)
{
	if (v > 0)
		w.writeBit(false);
	else
	{
		v = -v;
		w.writeBit(true);
	}

	if (v > 65535)
	{
		w.writeBit(true);
		w.writeUnsigned(31, v);
	}
	else
	{
		w.writeBit(false);
		w.writeUnsigned(16, v);
	}

	return true;
}

bool write_uint32(BitWriter& w, uint32_t v)
{
	if (v > 65535)
	{
		w.writeBit(true);
		w.writeUnsigned(32, v);
	}
	else
	{
		w.writeBit(false);
		w.writeUnsigned(16, v);
	}
	return true;
}

bool write_float(BitWriter& w, float v)
{
	w.writeUnsigned(32, *(uint32_t*)&v);
	return true;
}

bool read_string(BitReader& r, std::wstring& outString)
{
	uint16_t u8len;

	if (!read_uint16(r, u8len))
		return false;

	if (u8len > 0)
	{
		uint8_t* buf = (uint8_t*)alloca(u8len * sizeof(uint8_t) + u8len * sizeof(wchar_t) + 8);
		if (!buf)
			return false;

		uint8_t* u8str = buf;
		wchar_t* wstr = (wchar_t*)(buf + u8len * sizeof(uint8_t));
		wchar_t* wptr = wstr;

		for (uint16_t i = 0; i < u8len; ++i)
		{
			if (!read_uint8(r, u8str[i]))
				return false;
		}

		Utf8Encoding utf8enc;
		for (uint16_t i = 0; i < u8len; )
		{
			int n = utf8enc.translate(u8str + i, u8len - i, *wptr++);
			if (n <= 0)
				return false;
			i += n;
		}

		outString = std::wstring(wstr, wptr);
	}
	else
		outString.clear();

	return true;
}

bool write_string(BitWriter& w, const std::wstring& str)
{
	T_ASSERT (str.length() <= std::numeric_limits< uint16_t >::max());
	
	uint32_t length = uint32_t(str.length());
	if (length > 0)
	{
		uint8_t* u8str = (uint8_t*)alloca(length * 4);
		uint16_t u8len;
		
		Utf8Encoding utf8enc;
		u8len = utf8enc.translate(str.c_str(), length, u8str);

		if (!write_uint16(w, u8len))
			return false;

		for (uint16_t i = 0; i < u8len; ++i)
		{
			if (!write_uint8(w, u8str[i]))
				return false;
		}

		return true;
	}
	else
	{
		if (!write_uint16(w, 0))
			return false;

		return true;
	}
}

bool read_string(BitReader& r, std::string& outString)
{
	std::wstring ws;
	if (!read_string(r, ws))
		return false;

	outString = wstombs(ws);
	return true;
}

bool write_string(BitWriter& w, const std::string& str)
{
	std::wstring ws = mbstows(str);
	return write_string(w, ws);
}

template < typename AttributeType, typename MemberType >
const AttributeType* findAttribute(const MemberType& m)
{
	const Attribute* attributes = m.getAttributes();
	return attributes ? attributes->find< AttributeType >() : 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.CompactSerializer", CompactSerializer, Serializer);

CompactSerializer::CompactSerializer(IStream* stream, const TypeInfo** types)
:	m_types(types)
,	m_direction(stream->canRead() ? SdRead : SdWrite)
,	m_reader(stream)
,	m_writer(stream)
{
}

void CompactSerializer::flush()
{
	if (m_direction == SdWrite)
		m_writer.flush();
}

Serializer::Direction CompactSerializer::getDirection() const
{
	return m_direction;
}

bool CompactSerializer::operator >> (const Member< bool >& m)
{
	if (m_direction == SdRead)
		return read_bool(m_reader, m);
	else
		return write_bool(m_writer, m);
}

bool CompactSerializer::operator >> (const Member< int8_t >& m)
{
	if (m_direction == SdRead)
		return read_int8(m_reader, m);
	else
		return write_int8(m_writer, m);
}

bool CompactSerializer::operator >> (const Member< uint8_t >& m)
{
	if (m_direction == SdRead)
		return read_uint8(m_reader, m);
	else
		return write_uint8(m_writer, m);
}

bool CompactSerializer::operator >> (const Member< int16_t >& m)
{
	if (m_direction == SdRead)
		return read_int16(m_reader, m);
	else
		return write_int16(m_writer, m);
}

bool CompactSerializer::operator >> (const Member< uint16_t >& m)
{
	if (m_direction == SdRead)
		return read_uint16(m_reader, m);
	else
		return write_uint16(m_writer, m);
}

bool CompactSerializer::operator >> (const Member< int32_t >& m)
{
	if (m_direction == SdRead)
		return read_int32(m_reader, m);
	else
		return write_int32(m_writer, m);
}

bool CompactSerializer::operator >> (const Member< uint32_t >& m)
{
	if (m_direction == SdRead)
		return read_uint32(m_reader, m);
	else
		return write_uint32(m_writer, m);
}

bool CompactSerializer::operator >> (const Member< int64_t >& m)
{
	T_FATAL_ERROR;
	return false;
}

bool CompactSerializer::operator >> (const Member< uint64_t >& m)
{
	T_FATAL_ERROR;
	return false;
}

bool CompactSerializer::operator >> (const Member< float >& m)
{
	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		if (m_direction == SdRead)
		{
			uint8_t uv;
			if (!read_uint8(m_reader, uv))
				return false;

			m = uv * (range->getMax() - range->getMin()) / 255.0f + range->getMin();
			return true;
		}
		else
		{
			float v = clamp< float >(m, range->getMin(), range->getMax());
			uint8_t uv = uint8_t(255.0f * (v - range->getMin()) / (range->getMax() - range->getMin()));
			return write_uint8(m_writer, uv);
		}
	}
	else
	{
		if (m_direction == SdRead)
			return read_float(m_reader, m);
		else
			return write_float(m_writer, m);
	}
}

bool CompactSerializer::operator >> (const Member< double >& m)
{
	T_FATAL_ERROR;
	return false;
}

bool CompactSerializer::operator >> (const Member< std::string >& m)
{
	if (m_direction == SdRead)
		return read_string(m_reader, m);
	else
		return write_string(m_writer, m);
}

bool CompactSerializer::operator >> (const Member< std::wstring >& m)
{
	if (m_direction == SdRead)
		return read_string(m_reader, m);
	else
		return write_string(m_writer, m);
}

bool CompactSerializer::operator >> (const Member< Guid >& m)
{
	Guid& guid = m;
	if (m_direction == SdRead)
	{
		bool validGuid = false;
		if (!read_bool(m_reader, validGuid))
			return false;

		if (validGuid)
		{
			uint8_t data[16];
			for (uint32_t i = 0; i < 16; ++i)
			{
				if (!read_uint8(m_reader, data[i]))
					return false;
			}
			guid = Guid(data);
		}
		else
			guid = Guid();
	}
	else
	{
		bool validGuid = guid.isValid();
		if (!write_bool(m_writer, validGuid))
			return false;

		if (validGuid)
		{
			const uint8_t* data = static_cast< const uint8_t* >(guid);
			for (uint32_t i = 0; i < 16; ++i)
			{
				if (!write_uint8(m_writer, data[i]))
					return false;
			}
		}
	}
	return true;
}

bool CompactSerializer::operator >> (const Member< Path >& m)
{
	T_FATAL_ERROR;
	return false;
}

bool CompactSerializer::operator >> (const Member< Color4ub >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		result &= read_uint8(m_reader, m->r);
		result &= read_uint8(m_reader, m->g);
		result &= read_uint8(m_reader, m->b);
		result &= read_uint8(m_reader, m->a);
	}
	else
	{
		result &= write_uint8(m_writer, m->r);
		result &= write_uint8(m_writer, m->g);
		result &= write_uint8(m_writer, m->b);
		result &= write_uint8(m_writer, m->a);
	}
	return result;
}

bool CompactSerializer::operator >> (const Member< Color4f >& m)
{
	float T_MATH_ALIGN16 e[4];
	bool result = true;

	if (m_direction == SdRead)
	{
		for (uint32_t i = 0; i < 4; ++i)
			result &= read_float(m_reader, e[i]);
		(*m) = Color4f::loadUnaligned(e);
	}
	else
	{
		(*m).storeUnaligned(e);
		for (uint32_t i = 0; i < 4; ++i)
			result &= write_float(m_writer, e[i]);
	}

	return result;
}

bool CompactSerializer::operator >> (const Member< Scalar >& m)
{
	Scalar& v = m;
	if (m_direction == SdRead)
	{
		float tmp;
		if (!read_float(m_reader, tmp))
			return false;
		v = Scalar(tmp);
	}
	else
	{
		if (!write_float(m_writer, float(v)))
			return false;
	}
	return true;
}

bool CompactSerializer::operator >> (const Member< Vector2 >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		result &= read_float(m_reader, m->x);
		result &= read_float(m_reader, m->y);
	}
	else
	{
		result &= write_float(m_writer, m->x);
		result &= write_float(m_writer, m->y);
	}
	return result;
}

bool CompactSerializer::operator >> (const Member< Vector4 >& m)
{
	float T_MATH_ALIGN16 e[4];
	uint32_t count = 4;
	bool result = true;

	const AttributeDirection* direction = findAttribute< AttributeDirection >(m);
	if (direction)
	{
		// If unit direction then serialize in packed 24-bit form.
		if (direction->getUnit())
		{
			if (m_direction == SdRead)
			{
				uint8_t u[3];

				read_uint8(m_reader, u[0]);
				read_uint8(m_reader, u[1]);
				read_uint8(m_reader, u[2]);

				(*m) = unpackUnit(u);
				return true;
			}
			else
			{
				uint8_t u[3];
				packUnit(*m, u);

				write_uint8(m_writer, u[0]);
				write_uint8(m_writer, u[1]);
				write_uint8(m_writer, u[2]);

				return true;
			}
		}

		e[3] = 0.0f;
		count = 3;
	}

	const AttributePoint* point = findAttribute< AttributePoint >(m);
	if (point)
	{
		e[3] = 1.0f;
		count = 3;
	}

	if (m_direction == SdRead)
	{
		for (uint32_t i = 0; i < count; ++i)
			result &= read_float(m_reader, e[i]);
		(*m) = Vector4::loadUnaligned(e);
	}
	else
	{
		(*m).storeUnaligned(e);
		for (uint32_t i = 0; i < count; ++i)
			result &= write_float(m_writer, e[i]);
	}

	return result;
}

bool CompactSerializer::operator >> (const Member< Matrix33 >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		for (uint32_t i = 0; i < 3 * 3; ++i)
			result &= read_float(m_reader, m->m[i]);
	}
	else
	{
		for (uint32_t i = 0; i < 3 * 3; ++i)
			result &= write_float(m_writer, m->m[i]);
	}
	return result;
}

bool CompactSerializer::operator >> (const Member< Matrix44 >& m)
{
	float T_MATH_ALIGN16 e[16];
	bool result = true;

	if (m_direction == SdRead)
	{
		for (uint32_t i = 0; i < 4 * 4; ++i)
			result &= read_float(m_reader, e[i]);
		(*m) = Matrix44::loadUnaligned(e);
	}
	else
	{
		(*m).storeUnaligned(e);
		for (uint32_t i = 0; i < 4 * 4; ++i)
			result &= write_float(m_writer, e[i]);
	}

	return result;
}

bool CompactSerializer::operator >> (const Member< Quaternion >& m)
{
	float T_MATH_ALIGN16 e[4];
	bool result = true;

	if (m_direction == SdRead)
	{
		for (uint32_t i = 0; i < 4; ++i)
			result &= read_float(m_reader, e[i]);
		m->e = Vector4::loadUnaligned(e);
	}
	else
	{
		m->e.storeUnaligned(e);
		for (uint32_t i = 0; i < 4; ++i)
			result &= write_float(m_writer, e[i]);
	}

	return result;	
}

bool CompactSerializer::operator >> (const Member< ISerializable* >& m)
{
	if (m_direction == SdRead)
	{
		ISerializable* object = 0;

		uint8_t typeId = m_reader.readUnsigned(5);
		
		// Index into type table.
		if (
			typeId != 0x00 &&
			typeId != 0x1f
		)
		{
			const TypeInfo* type = m_types[typeId - 1];
			T_FATAL_ASSERT (type);

			if (!(object = checked_type_cast< ISerializable* >(type->createInstance())))
				return false;

			if (!serialize(object, type->getVersion()))
				return false;
		}

		// Explicit type name.
		else if (typeId == 0x1f)
		{
			std::wstring typeName;
			if (!read_string(m_reader, typeName))
				return false;

			const TypeInfo* type = TypeInfo::find(typeName);
			if (!type)
				return false;

			if (!(object = checked_type_cast< ISerializable* >(type->createInstance())))
				return false;

			if (!serialize(object, type->getVersion()))
				return false;
		}

		m = object;
	}
	else
	{
		ISerializable* object = *m;
		if (object)
		{
			const TypeInfo& type = type_of(object);

			// Find type in type table.
			uint8_t typeId = 0;
			while (m_types[typeId])
			{
				if (m_types[typeId] == &type)
					break;
				++typeId;
			}

			if (m_types[typeId] == &type)
			{
				m_writer.writeUnsigned(5, typeId + 1);
				if (!serialize(object, type.getVersion()))
					return false;
			}
			else
			{
				m_writer.writeUnsigned(5, 0x1f);
				if (!write_string(m_writer, type.getName()))
					return false;
				if (!serialize(object, type.getVersion()))
					return false;
			}
		}
		else
			m_writer.writeUnsigned(5, 0x00);
	}
	return true;
}

bool CompactSerializer::operator >> (const Member< void* >& m)
{
	T_FATAL_ERROR;
	return false;
}

bool CompactSerializer::operator >> (const MemberArray& m)
{
	if (m_direction == SdRead)
	{
		uint8_t size;
		if (!read_uint8(m_reader, size))
			return false;

		m.reserve(size, size);
		for (uint8_t i = 0; i < size; ++i)
		{
			if (!m.read(*this))
				return false;
		}
	}
	else
	{
		T_FATAL_ASSERT (m.size() < 255);

		uint8_t size = uint8_t(m.size());
		write_uint8(m_writer, size);

		for (uint32_t i = 0; i < size; ++i)
		{
			if (!m.write(*this))
				return false;
		}
	}
	return true;
}

bool CompactSerializer::operator >> (const MemberComplex& m)
{
	return m.serialize(*this);
}

bool CompactSerializer::operator >> (const MemberEnumBase& m)
{
	return this->operator >> (*(MemberComplex*)(&m));
}

	}
}
