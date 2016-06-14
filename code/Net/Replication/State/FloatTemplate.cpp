#include <limits>
#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Net/Replication/State/FloatValue.h"
#include "Net/Replication/State/FloatTemplate.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

float safeDeltaTime(float v)
{
	float av = std::abs(v);
	if (av < 1.0f/60.0f)
		return 1.0f/60.0f * sign(v);
	else if (av > 0.5f)
		return 1.0f * sign(v);
	else
		return v;
}

float cyclicLerp(float a, float b, float t, float mn, float mx)
{
	float bh = b + (mx - mn);
	float bl = b - (mx - mn);

	if (abs(bh - a) < abs(b - a))
	{
		float o = lerp(a, bh, t);
		if (o > mx)
			o -= (mx - mn);
		return o;
	}
	
	if (abs(bl - a) < abs(b - a))
	{
		float o = lerp(a, bl, t);
		if (o < mn)
			o += (mx - mn);
		return o;
	}

	return lerp(a, b, t);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.FloatTemplate", FloatTemplate, IValueTemplate)

FloatTemplate::FloatTemplate(float threshold)
:	m_threshold(threshold)
,	m_min(std::numeric_limits< float >::max())
,	m_max(-std::numeric_limits< float >::max())
,	m_precision(Ftp32)
{
}

FloatTemplate::FloatTemplate(float threshold, float min, float max, FloatTemplatePrecision precision, bool cyclic)
:	m_threshold(threshold)
,	m_min(min)
,	m_max(max)
,	m_precision(precision)
,	m_cyclic(cyclic)
{
}

const TypeInfo& FloatTemplate::getValueType() const
{
	return type_of< FloatValue >();
}

uint32_t FloatTemplate::getMaxPackedDataSize() const
{
	switch (m_precision)
	{
	case Ftp32:
		return 32;
	case Ftp16:
		return 16;
	case Ftp8:
		return 8;
	case Ftp4:
		return 4;
	}
	return 0;
}

void FloatTemplate::pack(BitWriter& writer, const IValue* V) const
{
	float f = *checked_type_cast< const FloatValue* >(V);
	switch (m_precision)
	{
	case Ftp32:
		writer.writeUnsigned(32, *(uint32_t*)&f);
		break;
	case Ftp16:
		{
			uint32_t uf = uint32_t(clamp((f - m_min) / (m_max - m_min), 0.0f, 1.0f) * 65534.0f + 0.5f);
			writer.writeUnsigned(16, uf);
		}
		break;
	case Ftp8:
		{
			uint32_t uf = uint32_t(clamp((f - m_min) / (m_max - m_min), 0.0f, 1.0f) * 254.0f + 0.5f);
			writer.writeUnsigned(8, uf);
		}
		break;
	case Ftp4:
		{
			uint32_t uf = uint32_t(clamp((f - m_min) / (m_max - m_min), 0.0f, 1.0f) * 14.0f + 0.5f);
			writer.writeUnsigned(4, uf);
		}
		break;
	}
}

Ref< const IValue > FloatTemplate::unpack(BitReader& reader) const
{
	switch (m_precision)
	{
	case Ftp32:
		{
			uint32_t u = reader.readUnsigned(32);
			return new FloatValue(*(float*)&u);
		}
	case Ftp16:
		{
			uint32_t uf = reader.readUnsigned(16);
			float f = (uf / 65534.0f) * (m_max - m_min) + m_min;
			return new FloatValue(f);
		}
	case Ftp8:
		{
			uint32_t uf = reader.readUnsigned(8);
			float f = (uf / 254.0f) * (m_max - m_min) + m_min;
			return new FloatValue(f);
		}
	case Ftp4:
		{
			uint32_t uf = reader.readUnsigned(4);
			float f = (uf / 14.0f) * (m_max - m_min) + m_min;
			return new FloatValue(f);
		}
	}
	return 0;
}

Ref< const IValue > FloatTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const
{
	float Fn2 = *checked_type_cast< const FloatValue* >(Vn2);
	float Fn1 = *checked_type_cast< const FloatValue* >(Vn1);
	float F0 = *checked_type_cast< const FloatValue* >(V0);

	if (isNanOrInfinite(Fn2) || isNanOrInfinite(Fn1) || isNanOrInfinite(F0))
		return 0;

	float dT_n1_0 = safeDeltaTime(T0 - Tn1);
	float dT_n2_n1 = safeDeltaTime(Tn1 - Tn2);

	if (T <= Tn2)
		return Vn2;

	float Fo = 0.0f;
	if (T <= Tn1)
	{
		if (m_cyclic)
			Fo = cyclicLerp(Fn2, Fn1, (T - Tn2) / dT_n2_n1, m_min, m_max);
		else
			Fo = lerp(Fn2, Fn1, (T - Tn2) / dT_n2_n1);
	}
	else
	{
		if (m_cyclic)
			Fo = cyclicLerp(Fn1, F0, (T - Tn1) / dT_n1_0, m_min, m_max);
		else
			Fo = lerp(Fn1, F0, (T - Tn1) / dT_n1_0);
	}

	if (m_min < m_max)
		Fo = clamp(Fo, m_min, m_max);

	return new FloatValue(Fo);
}

bool FloatTemplate::threshold(const IValue* Vn1, const IValue* V) const
{
	float Fn1 = *checked_type_cast< const FloatValue* >(Vn1);
	float F0 = *checked_type_cast< const FloatValue* >(V);
	return traktor::abs(Fn1 - F0) > m_threshold;
}

	}
}
