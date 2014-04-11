#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Math/Const.h"
#include "Core/Math/Half.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributePrecision.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/CompactSerializer.h"

namespace traktor
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

bool read_half(BitReader& r, half_t& value)
{
	value = r.readUnsigned(16);
	return true;
}

bool read_float(BitReader& r, float& value)
{
	uint32_t v = r.readUnsigned(32);
	value = *(float*)&v;
	return true;
}

bool read_double(BitReader& r, double& value)
{
	uint32_t lo = r.readUnsigned(32);
	uint32_t hi = r.readUnsigned(32);
	uint64_t v = (uint64_t(hi) << 32) | lo;
	value = *(double*)&v;
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

bool write_half(BitWriter& w, half_t v)
{
	w.writeUnsigned(16, v);
	return true;
}

bool write_float(BitWriter& w, float v)
{
	w.writeUnsigned(32, *(uint32_t*)&v);
	return true;
}

bool write_double(BitWriter& w, double v)
{
	uint64_t qw = *(uint64_t*)&v;
	uint32_t lo = uint32_t(qw);
	uint32_t hi = uint32_t(qw >> 32);
	w.writeUnsigned(32, lo);
	w.writeUnsigned(32, hi);
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

Vector4 unpackUnit(const uint8_t u[3])
{
	Vector4 v(
		u[0] / 127.0f - 1.0f,
		u[1] / 127.0f - 1.0f,
		u[2] / 127.0f - 1.0f
	);

	Scalar ln = v.length();
	if (ln > FUZZY_EPSILON)
		return v / ln;
	else
		return Vector4::zero();
}

void packUnit(const Vector4& u, uint8_t out[3])
{
	Vector4 un = u.normalized();

	float x = 0.0f, y = 0.0f, z = 0.0f;
	float dx = un.x() / 128.0f;
	float dy = un.y() / 128.0f;
	float dz = un.z() / 128.0f;

	float md = std::numeric_limits< float >::max();
	for (int32_t i = 0; i < 128; ++i)
	{
		x += dx;
		y += dy;
		z += dz;

		int32_t ix = int32_t((x * 0.5f + 0.5f) * 255.0f);
		int32_t iy = int32_t((y * 0.5f + 0.5f) * 255.0f);
		int32_t iz = int32_t((z * 0.5f + 0.5f) * 255.0f);

		T_ASSERT (ix >= 0 && ix <= 255);
		T_ASSERT (iy >= 0 && iy <= 255);
		T_ASSERT (iz >= 0 && iz <= 255);

		Vector4 v(
			ix / 127.0f - 1.0f,
			iy / 127.0f - 1.0f,
			iz / 127.0f - 1.0f
		);

		float D = (un * v.length() - v).length();
		if (D < md)
		{
			out[0] = ix;
			out[1] = iy;
			out[2] = iz;
			md = D;
		}
	}

#if defined(_DEBUG)
	Vector4 check = unpackUnit(out);
	Vector4 error = (check - un).absolute();
	Scalar E = horizontalAdd4(error);
	T_ASSERT (E < 0.01f);
#endif
}

template < typename AttributeType, typename MemberType >
const AttributeType* findAttribute(const MemberType& m)
{
	const Attribute* attributes = m.getAttributes();
	return attributes ? attributes->find< AttributeType >() : 0;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.CompactSerializer", CompactSerializer, Serializer);

#define T_CHECK_STATUS \
	if (failed()) return;

CompactSerializer::CompactSerializer(IStream* stream, const TypeInfo** types, uint32_t ntypes)
:	m_types(types)
,	m_ntypes(ntypes)
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

void CompactSerializer::operator >> (const Member< bool >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_bool(m_reader, m);
	else
		write_bool(m_writer, m);
}

void CompactSerializer::operator >> (const Member< int8_t >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_int8(m_reader, m);
	else
		write_int8(m_writer, m);
}

void CompactSerializer::operator >> (const Member< uint8_t >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_uint8(m_reader, m);
	else
		write_uint8(m_writer, m);
}

void CompactSerializer::operator >> (const Member< int16_t >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_int16(m_reader, m);
	else
		write_int16(m_writer, m);
}

void CompactSerializer::operator >> (const Member< uint16_t >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_uint16(m_reader, m);
	else
		write_uint16(m_writer, m);
}

void CompactSerializer::operator >> (const Member< int32_t >& m)
{
	if (m_direction == SdRead)
		read_int32(m_reader, m);
	else
		write_int32(m_writer, m);
}

void CompactSerializer::operator >> (const Member< uint32_t >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_uint32(m_reader, m);
	else
		write_uint32(m_writer, m);
}

void CompactSerializer::operator >> (const Member< int64_t >& m)
{
	T_FATAL_ERROR;
}

void CompactSerializer::operator >> (const Member< uint64_t >& m)
{
	T_FATAL_ERROR;
}

void CompactSerializer::operator >> (const Member< float >& m)
{
	T_CHECK_STATUS;

	const AttributeRange* range = findAttribute< AttributeRange >(m);
	if (range)
	{
		if (m_direction == SdRead)
		{
			uint8_t uv;
			if (!ensure(read_uint8(m_reader, uv)))
				return;

			m = uv * (range->getMax() - range->getMin()) / 255.0f + range->getMin();
		}
		else
		{
			float v = clamp< float >(m, range->getMin(), range->getMax());
			uint8_t uv = uint8_t(255.0f * (v - range->getMin()) / (range->getMax() - range->getMin()));
			write_uint8(m_writer, uv);
		}
	}
	else
	{
		if (m_direction == SdRead)
			read_float(m_reader, m);
		else
			write_float(m_writer, m);
	}
}

void CompactSerializer::operator >> (const Member< double >& m)
{
	if (m_direction == SdRead)
		read_double(m_reader, m);
	else
		write_double(m_writer, m);
}

void CompactSerializer::operator >> (const Member< std::string >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_string(m_reader, m);
	else
		write_string(m_writer, m);
}

void CompactSerializer::operator >> (const Member< std::wstring >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
		read_string(m_reader, m);
	else
		write_string(m_writer, m);
}

void CompactSerializer::operator >> (const Member< Guid >& m)
{
	T_CHECK_STATUS;

	Guid& guid = m;
	if (m_direction == SdRead)
	{
		bool validGuid = false;
		if (!ensure(read_bool(m_reader, validGuid)))
			return;

		if (validGuid)
		{
			uint8_t data[16];
			for (uint32_t i = 0; i < 16; ++i)
				read_uint8(m_reader, data[i]);
			guid = Guid(data);
		}
		else
			guid = Guid();
	}
	else
	{
		bool validGuid = guid.isValid();
		if (!ensure(write_bool(m_writer, validGuid)))
			return;

		if (validGuid)
		{
			const uint8_t* data = static_cast< const uint8_t* >(guid);
			for (uint32_t i = 0; i < 16; ++i)
				write_uint8(m_writer, data[i]);
		}
	}
}

void CompactSerializer::operator >> (const Member< Path >& m)
{
	T_FATAL_ERROR;
}

void CompactSerializer::operator >> (const Member< Color4ub >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
	{
		read_uint8(m_reader, m->r);
		read_uint8(m_reader, m->g);
		read_uint8(m_reader, m->b);
		read_uint8(m_reader, m->a);
	}
	else
	{
		write_uint8(m_writer, m->r);
		write_uint8(m_writer, m->g);
		write_uint8(m_writer, m->b);
		write_uint8(m_writer, m->a);
	}
}

void CompactSerializer::operator >> (const Member< Color4f >& m)
{
	T_CHECK_STATUS;

	float T_MATH_ALIGN16 e[4];
	if (m_direction == SdRead)
	{
		for (uint32_t i = 0; i < 4; ++i)
			read_float(m_reader, e[i]);
		(*m) = Color4f::loadUnaligned(e);
	}
	else
	{
		(*m).storeUnaligned(e);
		for (uint32_t i = 0; i < 4; ++i)
			write_float(m_writer, e[i]);
	}
}

void CompactSerializer::operator >> (const Member< Scalar >& m)
{
	T_CHECK_STATUS;

	Scalar& v = m;
	if (m_direction == SdRead)
	{
		float tmp;
		read_float(m_reader, tmp);
		v = Scalar(tmp);
	}
	else
		write_float(m_writer, float(v));
}

void CompactSerializer::operator >> (const Member< Vector2 >& m)
{
	T_CHECK_STATUS;

	if (m_direction == SdRead)
	{
		read_float(m_reader, m->x);
		read_float(m_reader, m->y);
	}
	else
	{
		write_float(m_writer, m->x);
		write_float(m_writer, m->y);
	}
}

void CompactSerializer::operator >> (const Member< Vector4 >& m)
{
	T_CHECK_STATUS;

	AttributePrecision::PrecisionType precision = AttributePrecision::AtFull;
	float T_MATH_ALIGN16 e[4];
	uint32_t count = 4;

	const AttributeDirection* attrDirection = findAttribute< AttributeDirection >(m);
	if (attrDirection)
	{
		// If unit direction then serialize in packed 24-bit form.
		if (attrDirection->getUnit())
		{
			if (m_direction == SdRead)
			{
				uint8_t u[3];

				read_uint8(m_reader, u[0]);
				read_uint8(m_reader, u[1]);
				read_uint8(m_reader, u[2]);

				(*m) = unpackUnit(u);
				return;
			}
			else
			{
				uint8_t u[3];
				packUnit(*m, u);

				write_uint8(m_writer, u[0]);
				write_uint8(m_writer, u[1]);
				write_uint8(m_writer, u[2]);

				return;
			}
		}

		e[3] = 0.0f;
		count = 3;
	}

	const AttributePoint* attrPoint = findAttribute< AttributePoint >(m);
	if (attrPoint)
	{
		e[3] = 1.0f;
		count = 3;
	}

	const AttributePrecision* attrPrecision = findAttribute< AttributePrecision >(m);
	if (attrPrecision)
		precision = attrPrecision->getPrecision();

	if (m_direction == SdRead)
	{
		if (precision == AttributePrecision::AtFull)
		{
			for (uint32_t i = 0; i < count; ++i)
				read_float(m_reader, e[i]);
		}
		else if (precision == AttributePrecision::AtHalf)
		{
			for (uint32_t i = 0; i < count; ++i)
			{
				half_t he;
				read_half(m_reader, he);
				e[i] = halfToFloat(he);
			}
		}
		(*m) = Vector4::loadUnaligned(e);
	}
	else
	{
		(*m).storeUnaligned(e);
		if (precision == AttributePrecision::AtFull)
		{
			for (uint32_t i = 0; i < count; ++i)
				write_float(m_writer, e[i]);
		}
		else if (precision == AttributePrecision::AtHalf)
		{
			for (uint32_t i = 0; i < count; ++i)
			{
				half_t he = floatToHalf(e[i]);
				write_half(m_writer, he);
			}
		}
	}
}

void CompactSerializer::operator >> (const Member< Matrix33 >& m)
{
	T_CHECK_STATUS;
	if (m_direction == SdRead)
	{
		for (uint32_t i = 0; i < 3 * 3; ++i)
			read_float(m_reader, m->m[i]);
	}
	else
	{
		for (uint32_t i = 0; i < 3 * 3; ++i)
			write_float(m_writer, m->m[i]);
	}
}

void CompactSerializer::operator >> (const Member< Matrix44 >& m)
{
	T_CHECK_STATUS;

	float T_MATH_ALIGN16 e[16];
	if (m_direction == SdRead)
	{
		for (uint32_t i = 0; i < 4 * 4; ++i)
			read_float(m_reader, e[i]);
		(*m) = Matrix44::loadUnaligned(e);
	}
	else
	{
		(*m).storeUnaligned(e);
		for (uint32_t i = 0; i < 4 * 4; ++i)
			write_float(m_writer, e[i]);
	}
}

void CompactSerializer::operator >> (const Member< Quaternion >& m)
{
	T_CHECK_STATUS;

	float T_MATH_ALIGN16 e[4];
	if (m_direction == SdRead)
	{
		for (uint32_t i = 0; i < 4; ++i)
			read_float(m_reader, e[i]);
		m->e = Vector4::loadUnaligned(e);
	}
	else
	{
		m->e.storeUnaligned(e);
		for (uint32_t i = 0; i < 4; ++i)
			write_float(m_writer, e[i]);
	}
}

void CompactSerializer::operator >> (const Member< ISerializable* >& m)
{
	T_CHECK_STATUS;

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
			if (typeId - 1 >= m_ntypes)
				return;

			const TypeInfo* type = m_types[typeId - 1];
			if (!ensure(type != 0))
				return;

			if (!(object = checked_type_cast< ISerializable* >(type->createInstance())))
				return;

			serialize(object, type->getVersion());
		}

		// Explicit type name.
		else if (typeId == 0x1f)
		{
			std::wstring typeName;
			if (!read_string(m_reader, typeName))
				return;

			const TypeInfo* type = TypeInfo::find(typeName);
			if (!ensure(type != 0))
				return;

			object = checked_type_cast< ISerializable* >(type->createInstance());
			if (!ensure(object != 0))
				return;

			serialize(object, type->getVersion());
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
			while (m_types && typeId < m_ntypes && m_types[typeId])
			{
				if (m_types[typeId] == &type)
					break;
				++typeId;
			}

			if (m_types && typeId < m_ntypes && m_types[typeId] == &type)
			{
				m_writer.writeUnsigned(5, typeId + 1);
				serialize(object, type.getVersion());
			}
			else
			{
				m_writer.writeUnsigned(5, 0x1f);
				write_string(m_writer, type.getName());
				serialize(object, type.getVersion());
			}
		}
		else
			m_writer.writeUnsigned(5, 0x00);
	}
}

void CompactSerializer::operator >> (const Member< void* >& m)
{
	T_CHECK_STATUS;

	if (m_direction == SdRead)
	{
		uint32_t size;
			
		if (!ensure(read_uint32(m_reader, size)))
			return;
		
		if (!ensure(size <= m.getBlobSize()))
			return;

		m.setBlobSize(size);

		if (size > 0)
		{
			m_reader.alignByte();
			if (!ensure(m_reader.getStream()->read(m.getBlob(), size) == size))
				return;
		}
	}
	else
	{
		uint32_t size = m.getBlobSize();

		if (!ensure(write_uint32(m_writer, size)))
			return;

		if (size > 0)
		{
			m_writer.flush();
			if (!ensure(m_writer.getStream()->write(m.getBlob(), size) == size))
				return;
		}
	}
}

void CompactSerializer::operator >> (const MemberArray& m)
{
	T_CHECK_STATUS;

	if (m_direction == SdRead)
	{
		uint32_t size;
		if (!ensure(read_uint32(m_reader, size)))
			return;

		m.reserve(size, size);
		for (uint32_t i = 0; i < size; ++i)
			m.read(*this);
	}
	else
	{
		uint32_t size = uint32_t(m.size());
		if (!ensure(write_uint32(m_writer, size)))
			return;

		for (uint32_t i = 0; i < size; ++i)
			m.write(*this);
	}
}

void CompactSerializer::operator >> (const MemberComplex& m)
{
	T_CHECK_STATUS;
	m.serialize(*this);
}

void CompactSerializer::operator >> (const MemberEnumBase& m)
{
	T_CHECK_STATUS;
	this->operator >> (*(MemberComplex*)(&m));
}

}
