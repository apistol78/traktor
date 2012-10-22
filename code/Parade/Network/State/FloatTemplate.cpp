#include <limits>
#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Parade/Network/State/FloatValue.h"
#include "Parade/Network/State/FloatTemplate.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

const float c_idleThreshold = 1e-4f;
const float c_idleThresholdLowPrecision = 1.0f / 256.0f;

float safeDenom(float v)
{
	if (abs(v) < FUZZY_EPSILON)
		return FUZZY_EPSILON * sign(v);
	else
		return v;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.FloatTemplate", FloatTemplate, IValueTemplate)

FloatTemplate::FloatTemplate()
:	m_min(std::numeric_limits< float >::max())
,	m_max(-std::numeric_limits< float >::max())
,	m_idle(0.0f)
,	m_lowPrecision(false)
,	m_haveIdle(false)
{
}

FloatTemplate::FloatTemplate(float min, float max, bool lowPrecision)
:	m_min(min)
,	m_max(max)
,	m_idle(0.0f)
,	m_lowPrecision(lowPrecision)
,	m_haveIdle(false)
{
}

FloatTemplate::FloatTemplate(float min, float max, float idle, bool lowPrecision)
:	m_min(min)
,	m_max(max)
,	m_idle(idle)
,	m_lowPrecision(lowPrecision)
,	m_haveIdle(true)
{
}

void FloatTemplate::pack(BitWriter& writer, const IValue* V) const
{
	float f = *checked_type_cast< const FloatValue* >(V);

	if (m_haveIdle)
	{
		bool idle = false;
		
		if (!m_lowPrecision)
			idle = bool(abs(f - m_idle) <= c_idleThreshold);
		else
			idle = bool(abs(f - m_idle) <= c_idleThresholdLowPrecision);

		writer.writeBit(idle);
		if (idle)
			return;
	}

	if (!m_lowPrecision)
		writer.writeUnsigned(32, *(uint32_t*)&f);
	else
	{
		uint8_t uf = uint8_t(clamp((f - m_min) / (m_max - m_min), 0.0f, 1.0f) * 255);
		writer.writeUnsigned(8, uf);
	}
}

Ref< const IValue > FloatTemplate::unpack(BitReader& reader) const
{
	if (m_haveIdle)
	{
		bool idle = reader.readBit();
		if (idle)
			return new FloatValue(m_idle);
	}

	if (!m_lowPrecision)
	{
		uint32_t u = reader.readUnsigned(32);
		return new FloatValue(*(float*)&u);
	}
	else
	{
		uint8_t uf = reader.readUnsigned(8);
		float f = (uf / 255.0f) * (m_max - m_min) + m_min;
		return new FloatValue(f);
	}
}

Ref< const IValue > FloatTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, const IValue* V, float T) const
{
	float fn1 = *checked_type_cast< const FloatValue* >(Vn1);
	float f0 = *checked_type_cast< const FloatValue* >(V0);

	float dT_n2_n1 = safeDenom(Tn1 - Tn2);
	float dT_n1_0 = safeDenom(T0 - Tn1);
	
	if (Vn2)
	{
		float fn2 = *checked_type_cast< const FloatValue* >(Vn2);

		float v2_1 = (fn1 - fn2) / dT_n2_n1;
		float v1_0 = (f0 - fn1) / dT_n1_0;
		float a = clamp((v1_0 - v2_1) / dT_n1_0, -1.0f, 1.0f);
		float f = f0 + (f0 - fn1) * (T - T0) + 0.5f * a * (T - T0) * (T - T0);

		if (m_min < m_max)
			f = clamp(f, m_min, m_max);

		return new FloatValue(f);
	}
	else
	{
		float k = (T - Tn1) / dT_n1_0;
		float f = f0 + (fn1 - f0) * k;

		if (m_min < m_max)
			f = clamp(f, m_min, m_max);

		return new FloatValue(f);
	}
}

	}
}
