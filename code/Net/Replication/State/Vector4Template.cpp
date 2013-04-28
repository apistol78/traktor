#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Net/Replication/Pack.h"
#include "Net/Replication/State/Config.h"
#include "Net/Replication/State/Vector4Value.h"
#include "Net/Replication/State/Vector4Template.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

void packScalar(BitWriter& writer, float v)
{
	uint16_t iv = uint16_t(clamp(v * 0.5f + 0.5f, 0.0f, 1.0f) * 65535.0f);
	writer.writeUnsigned(16, iv);
}

float unpackScalar(BitReader& reader)
{
	uint16_t iv = reader.readUnsigned(16);
	return (iv / 65535.0f) * 2.0f - 1.0f;
}

float errorV4(const Vector4& Vl, const Vector4& Vr)
{
	return (Vl - Vr).length();
}

float safeSqrt(float v)
{
	if (v > FUZZY_EPSILON)
		return std::sqrt(v);
	else
		return 0.0f;
}

float safeDeltaTime(float v)
{
	float av = std::abs(v);
	if (av < 1e-5f)
		return 1e-5f * sign(v);
	else if (av > 1.0f)
		return 1.0f * sign(v);
	else
		return v;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Vector4Template", Vector4Template, IValueTemplate)

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

			uint8_t u[3];
			packUnit(v, u);

			writer.writeUnsigned(8, u[0]);
			writer.writeUnsigned(8, u[1]);
			writer.writeUnsigned(8, u[2]);

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
			uint8_t u[3];
			u[0] = reader.readUnsigned(8);
			u[1] = reader.readUnsigned(8);
			u[2] = reader.readUnsigned(8);
			v = unpackUnit(u);

			float ln2 = reader.readUnsigned(16) / 512.0f;
			v *= Scalar(ln2);

			v = v.xyz0() + Vector4(0.0f, 0.0f, 0.0f, m_w);
		}
		return new Vector4Value(v);
	}
}

float Vector4Template::error(const IValue* Vl, const IValue* Vr) const
{
	Vector4 vl = *checked_type_cast< const Vector4Value* >(Vl);
	Vector4 vr = *checked_type_cast< const Vector4Value* >(Vr);
	return errorV4(vl, vr) * c_errorScaleContinuous;
}

Ref< const IValue > Vector4Template::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, const IValue* V, float T) const
{
	Vector4 Fn1 = *checked_type_cast< const Vector4Value* >(Vn1);
	Vector4 F0 = *checked_type_cast< const Vector4Value* >(V0);
	Vector4 Fc = *checked_type_cast< const Vector4Value* >(V);
	Vector4 Ff;

	float dT_n2_n1 = safeDeltaTime(Tn1 - Tn2);
	float dT_n1_0 = safeDeltaTime(T0 - Tn1);

	{
		float k = (T - T0) / dT_n1_0;
		Ff = F0 + (F0 - Fn1) * Scalar(k);
	}

	// If current simulated state is known then blend into it if last known
	// state is becoming too old or extrapolated too far away.
	float k_T = clamp((T - T0) / c_maxRubberBandTime, 0.0f, 0.9f);
	Ff = lerp(Ff, Fc, Scalar(k_T));

	return new Vector4Value(Ff);
}

	}
}
