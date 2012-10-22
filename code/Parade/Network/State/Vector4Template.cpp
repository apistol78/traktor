#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Math/Const.h"
#include "Parade/Network/State/Vector4Value.h"
#include "Parade/Network/State/Vector4Template.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

void packUnit(BitWriter& writer, float v)
{
	uint16_t iv = uint16_t(clamp(v * 0.5f + 0.5f, 0.0f, 1.0f) * 65535.0f);
	writer.writeUnsigned(16, iv);
}

float unpackUnit(BitReader& reader)
{
	uint16_t iv = reader.readUnsigned(16);
	return (iv / 65535.0f) * 2.0f - 1.0f;
}

float safeSqrt(float v)
{
	if (v > 0.0f)
		return std::sqrt(v);
	else
		return 0.0f;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.Vector4Template", Vector4Template, IValueTemplate)

Vector4Template::Vector4Template()
:	m_w(0.0f)
,	m_lowPrecision(false)
,	m_immutableW(false)
{
}

Vector4Template::Vector4Template(float w, bool lowPrecision)
:	m_w(w)
,	m_lowPrecision(lowPrecision)
,	m_immutableW(true)
{
}

void Vector4Template::pack(BitWriter& writer, const IValue* V) const
{
	Vector4 v = *checked_type_cast< const Vector4Value* >(V);
	if (!m_lowPrecision)
	{
		float e[4];
		v.storeUnaligned(e);

		writer.writeUnsigned(32, *(uint32_t*)&e[0]);
		writer.writeUnsigned(32, *(uint32_t*)&e[1]);
		writer.writeUnsigned(32, *(uint32_t*)&e[2]);

		if (!m_immutableW)
			writer.writeUnsigned(32, *(uint32_t*)&e[3]);
	}
	else
	{
		float ln = v.length();
		if (abs(ln) > 1.0f/512.0f)
		{
			// 1
			writer.writeBit(true);

			v /= Scalar(ln);

			// 2 * 16
			packUnit(writer, v.x());
			packUnit(writer, v.y());

			// 1
			writer.writeBit(v.z() < 0.0f);

			// 16
			uint32_t uln = std::min< uint32_t >(uint32_t(ln * 512.0f), 65535);
			writer.writeUnsigned(16, uln);
		}
		else
			// 1
			writer.writeBit(false);
	}
}

Ref< const IValue > Vector4Template::unpack(BitReader& reader) const
{
	if (!m_lowPrecision)
	{
		uint32_t ux = reader.readUnsigned(32);
		uint32_t uy = reader.readUnsigned(32);
		uint32_t uz = reader.readUnsigned(32);

		if (!m_immutableW)
		{
			uint32_t uw = reader.readUnsigned(32);
			return new Vector4Value(Vector4(
				*(float*)&ux,
				*(float*)&uy,
				*(float*)&uz,
				*(float*)&uw
			));
		}
		else
		{
			return new Vector4Value(Vector4(
				*(float*)&ux,
				*(float*)&uy,
				*(float*)&uz,
				m_w
			));
		}
	}
	else
	{
		Vector4 v = Vector4::zero();
		if (reader.readBit())
		{
			float x = unpackUnit(reader);
			float y = unpackUnit(reader);
			float z = safeSqrt(1.0f - x * x - y * y);

			if (reader.readBit())
				z = -z;
		
			v = Vector4(x, y, z, m_w);

			Scalar ln1 = v.length();
			if (ln1 > FUZZY_EPSILON)
				v /= ln1;

			float ln2 = reader.readUnsigned(16) / 512.0f;
			v *= Scalar(ln2);
		}
		return new Vector4Value(v);
	}
}

Ref< const IValue > Vector4Template::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, const IValue* V, float T) const
{
	Vector4 fn1 = *checked_type_cast< const Vector4Value* >(Vn1);
	Vector4 f0 = *checked_type_cast< const Vector4Value* >(V0);

	if (T0 > Tn1 + FUZZY_EPSILON)
	{
		float k = (T - Tn1) / (T0 - Tn1);
		return new Vector4Value(f0 + (fn1 - f0) * Scalar(k));
	}
	else
		return new Vector4Value(f0);
}

	}
}
