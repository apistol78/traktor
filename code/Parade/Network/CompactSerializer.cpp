#include "Core/Io/IStream.h"
#include "Core/Math/Const.h"
#include "Parade/Network/CompactSerializer.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

bool readU16(BitReader& r, uint16_t& outV)
{
	if (r.readBit())
		outV = r.readUnsigned(16);
	else
		outV = r.readUnsigned(8);
	return true;
}

bool writeU16(BitWriter& w, uint16_t v)
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

bool readF32(BitReader& r, float& outV)
{
	//bool z = r.readBit();
	//if (z)
	//{
	//	outV = 0.0f;
	//	return true;
	//}

	//bool o = r.readBit();
	//if (o)
	//{
	//	bool sign = r.readBit();
	//	uint32_t iv = r.readUnsigned(8);
	//	outV = float(iv) / 255.0f * (sign ? 1.0f : -1.0f);
	//	return true;
	//}

	uint32_t v = r.readUnsigned(32);
	outV = *(float*)&v;

	return true;
}

bool writeF32(BitWriter& w, float v)
{
	//if (std::abs(v) < FUZZY_EPSILON)
	//{
	//	w.writeBit(true);
	//	return true;
	//}

	//w.writeBit(false);

	//if (std::abs(v) <= 1.0f)
	//{
	//	bool sign = v > 0.0f;
	//	uint32_t iv = uint32_t(std::abs(v) * 255.0f);
	//	float err = std::abs(v - float(iv) / 255.0f * (sign ? 1.0f : -1.0f));
	//	if (err <= 0.0005f)
	//	{
	//		w.writeBit(true);
	//		w.writeBit(sign);
	//		w.writeUnsigned(8, iv);
	//		return true;
	//	}
	//}

	//w.writeBit(false);

	w.writeUnsigned(32, *(uint32_t*)&v);
	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.CompactSerializer", CompactSerializer, Serializer);

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
		m = m_reader.readBit();
	else
		m_writer.writeBit(m);
	return true;
}

bool CompactSerializer::operator >> (const Member< int8_t >& m)
{
	if (m_direction == SdRead)
		m = m_reader.readSigned(8);
	else
		m_writer.writeSigned(8, m);
	return true;
}

bool CompactSerializer::operator >> (const Member< uint8_t >& m)
{
	if (m_direction == SdRead)
		m = m_reader.readUnsigned(8);
	else
		m_writer.writeUnsigned(8, m);
	return true;
}

bool CompactSerializer::operator >> (const Member< int16_t >& m)
{
	if (m_direction == SdRead)
		m = m_reader.readSigned(16);
	else
		m_writer.writeSigned(16, m);
	return true;
}

bool CompactSerializer::operator >> (const Member< uint16_t >& m)
{
	if (m_direction == SdRead)
		m = m_reader.readUnsigned(16);
	else
		m_writer.writeUnsigned(16, m);
	return true;
}

bool CompactSerializer::operator >> (const Member< int32_t >& m)
{
	if (m_direction == SdRead)
		m = m_reader.readSigned(32);
	else
		m_writer.writeSigned(32, m);
	return true;
}

bool CompactSerializer::operator >> (const Member< uint32_t >& m)
{
	if (m_direction == SdRead)
		m = m_reader.readUnsigned(32);
	else
		m_writer.writeUnsigned(32, m);
	return true;
}

bool CompactSerializer::operator >> (const Member< int64_t >& m)
{
	return false;
}

bool CompactSerializer::operator >> (const Member< uint64_t >& m)
{
	return false;
}

bool CompactSerializer::operator >> (const Member< float >& m)
{
	if (m_direction == SdRead)
		return readF32(m_reader, m);
	else
		return writeF32(m_writer, m);
}

bool CompactSerializer::operator >> (const Member< double >& m)
{
	return false;
}

bool CompactSerializer::operator >> (const Member< std::string >& m)
{
	return false;
}

bool CompactSerializer::operator >> (const Member< std::wstring >& m)
{
	return false;
}

bool CompactSerializer::operator >> (const Member< Guid >& m)
{
	return false;
}

bool CompactSerializer::operator >> (const Member< Path >& m)
{
	return false;
}

bool CompactSerializer::operator >> (const Member< Color4ub >& m)
{
	return false;
}

bool CompactSerializer::operator >> (const Member< Scalar >& m)
{
	Scalar& v = m;
	if (m_direction == SdRead)
	{
		float tmp;
		if (!readF32(m_reader, tmp))
			return false;
		v = Scalar(tmp);
	}
	else
	{
		if (!writeF32(m_writer, float(v)))
			return false;
	}
	return true;
}

bool CompactSerializer::operator >> (const Member< Vector2 >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		result &= readF32(m_reader, m->x);
		result &= readF32(m_reader, m->y);
	}
	else
	{
		result &= writeF32(m_writer, m->x);
		result &= writeF32(m_writer, m->y);
	}
	return result;
}

bool CompactSerializer::operator >> (const Member< Vector4 >& m)
{
	float T_MATH_ALIGN16 e[4];
	bool result = true;

	if (m_direction == SdRead)
	{
		for (uint32_t i = 0; i < 4; ++i)
			result &= readF32(m_reader, e[i]);
		(*m) = Vector4::loadAligned(e);
	}
	else
	{
		(*m).storeAligned(e);
		for (uint32_t i = 0; i < 4; ++i)
			result &= writeF32(m_writer, e[i]);
	}

	return result;
}

bool CompactSerializer::operator >> (const Member< Matrix33 >& m)
{
	bool result = true;
	if (m_direction == SdRead)
	{
		for (int i = 0; i < 3 * 3; ++i)
			result &= readF32(m_reader, m->m[i]);
	}
	else
	{
		for (int i = 0; i < 3 * 3; ++i)
			result &= writeF32(m_writer, m->m[i]);
	}
	return result;
}

bool CompactSerializer::operator >> (const Member< Matrix44 >& m)
{
	float T_MATH_ALIGN16 e[16];
	bool result = true;

	if (m_direction == SdRead)
	{
		for (uint32_t i = 0; i < 16; ++i)
			result &= readF32(m_reader, e[i]);
		(*m) = Matrix44::loadAligned(e);
	}
	else
	{
		(*m).storeAligned(e);
		for (uint32_t i = 0; i < 16; ++i)
			result &= writeF32(m_writer, e[i]);
	}

	return result;
}

bool CompactSerializer::operator >> (const Member< Quaternion >& m)
{
	float T_MATH_ALIGN16 e[4];
	bool result;

	if (m_direction == SdRead)
	{
		for (uint32_t i = 0; i < 4; ++i)
			result &= readF32(m_reader, e[i]);
		m->e = Vector4::loadAligned(e);
	}
	else
	{
		m->e.storeAligned(e);
		for (uint32_t i = 0; i < 4; ++i)
			result &= writeF32(m_writer, e[i]);
	}

	return result;	
}

bool CompactSerializer::operator >> (const Member< ISerializable* >& m)
{
	if (m_direction == SdRead)
	{
		ISerializable* object = 0;

		uint8_t typeId = m_reader.readUnsigned(4);
		if (typeId > 0)
		{
			const TypeInfo* type = m_types[typeId - 1];
			T_ASSERT (type);

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

			uint8_t typeId = 0;
			for (;;)
			{
				T_ASSERT_M (m_types[typeId], L"Type not known by CompactSerializer");
				if (m_types[typeId] == &type)
					break;
				++typeId;
			}

			m_writer.writeUnsigned(4, typeId + 1);

			if (!serialize(object, type.getVersion()))
				return false;
		}
		else
			m_writer.writeUnsigned(4, 0);
	}
	return true;
}

bool CompactSerializer::operator >> (const Member< void* >& m)
{
	return false;
}

bool CompactSerializer::operator >> (const MemberArray& m)
{
	if (m_direction == SdRead)
	{
		uint16_t size;
		readU16(m_reader, size);

		m.reserve(size, size);
		for (uint16_t i = 0; i < size; ++i)
		{
			if (!m.read(*this))
				return false;
		}
	}
	else
	{
		uint16_t size = uint16_t(m.size());
		writeU16(m_writer, size);

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
