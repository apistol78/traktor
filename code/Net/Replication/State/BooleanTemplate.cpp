#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Math/MathUtils.h"
#include "Net/Replication/State/BooleanValue.h"
#include "Net/Replication/State/BooleanTemplate.h"

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
	else if (av > 1.0f)
		return 1.0f * sign(v);
	else
		return v;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.BooleanTemplate", BooleanTemplate, IValueTemplate)

BooleanTemplate::BooleanTemplate(const wchar_t* const tag, float threshold)
:	m_tag(tag)
,	m_threshold(threshold)
{
}

const TypeInfo& BooleanTemplate::getValueType() const
{
	return type_of< BooleanValue >();
}

uint32_t BooleanTemplate::getMaxPackedDataSize() const
{
	return 1;
}

void BooleanTemplate::pack(BitWriter& writer, const IValue* V) const
{
	bool f = *checked_type_cast< const BooleanValue* >(V);
	writer.writeBit(f);
}

Ref< const IValue > BooleanTemplate::unpack(BitReader& reader) const
{
	bool f = reader.readBit();
	return new BooleanValue(f);
}

Ref< const IValue > BooleanTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const
{
	bool Fn2 = *checked_type_cast< const BooleanValue* >(Vn2);
	bool Fn1 = *checked_type_cast< const BooleanValue* >(Vn1);
	bool F0 = *checked_type_cast< const BooleanValue* >(V0);

	float dT_n1_0 = safeDeltaTime(T0 - Tn1);
	float dT_n2_n1 = safeDeltaTime(Tn1 - Tn2);

	if (T <= Tn2)
		return Vn2;

	if (T <= Tn1)
	{
		float k = (T - Tn2) / dT_n2_n1;
		return new BooleanValue(k >= m_threshold ? Fn2 : Fn1);
	}

	if (T <= T0)
	{
		float k = (T - Tn1) / dT_n1_0;
		return new BooleanValue(k >= m_threshold ? Fn1 : F0);
	}

	return new BooleanValue(F0);
}

bool BooleanTemplate::threshold(const IValue* Vn1, const IValue* V) const
{
	return false;
}

	}
}
