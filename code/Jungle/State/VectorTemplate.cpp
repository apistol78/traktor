/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Jungle/State/VectorValue.h"
#include "Jungle/State/VectorTemplate.h"

namespace traktor::jungle
{
	namespace
	{

float safeDeltaTime(float v)
{
	const float av = std::abs(v);
	if (av < 1.0f/60.0f)
		return 1.0f/60.0f * sign(v);
	else if (av > 1.0f)
		return 1.0f * sign(v);
	else
		return v;
}

Vector4 interpolate(const Vector4& t0, float T0, const Vector4& t1, float T1, float T)
{
	return lerp(t0, t1, Scalar((T - T0) / safeDeltaTime(T1 - T0)));
}

template < int32_t IntBits, int32_t FractBits >
class GenericFixedPoint
{
	const static int32_t ms_factor = 1 << FractBits;

public:
	GenericFixedPoint(int32_t v)
	:	m_v(v)
	{
	}

	GenericFixedPoint(float v)
	:	m_v(int32_t(v * ms_factor))
	{
	}

	int32_t raw() const { return m_v; }

	operator float () const { return float(m_v) / ms_factor; }

private:
	int32_t m_v;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.jungle.VectorTemplate", VectorTemplate, IValueTemplate)

VectorTemplate::VectorTemplate(const std::wstring& tag)
:	m_tag(tag)
{
}

const TypeInfo& VectorTemplate::getValueType() const
{
	return type_of< VectorValue >();
}

uint32_t VectorTemplate::getMaxPackedDataSize() const
{
	return 4 * (13+11);
}

void VectorTemplate::pack(BitWriter& writer, const IValue* V) const
{
	const Vector4 v = *mandatory_non_null_type_cast< const VectorValue* >(V);
	float T_MATH_ALIGN16 e[4];

	// 4 * (13+11)
	v.storeAligned(e);
	for (uint32_t i = 0; i < 4; ++i)
		writer.writeSigned(13+11, GenericFixedPoint< 13, 11 >(e[i]).raw());
}

Ref< const IValue > VectorTemplate::unpack(BitReader& reader) const
{
	float T_MATH_ALIGN16 f[4];
	for (uint32_t i = 0; i < 4; ++i)
	{
		f[i] = GenericFixedPoint< 13, 11 >(reader.readSigned(13+11));
		T_ASSERT(!isNanOrInfinite(f[i]));
	}
	return new VectorValue(Vector4::loadAligned(f));
}

Ref< const IValue > VectorTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const
{
	const Vector4 Sn2 = *mandatory_non_null_type_cast< const VectorValue* >(Vn2);
	const Vector4 Sn1 = *mandatory_non_null_type_cast< const VectorValue* >(Vn1);
	const Vector4 S0 = *mandatory_non_null_type_cast< const VectorValue* >(V0);

	Scalar dT_0(safeDeltaTime(T - T0));
	Scalar dT_n1_0(safeDeltaTime(T0 - Tn1));
	Scalar dT_n2_n1(safeDeltaTime(Tn1 - Tn2));

	if (T <= Tn2)
		return Vn2;

	if (T <= Tn1)
	{
		return new VectorValue(
			interpolate(Sn2, Tn2, Sn1, Tn1, T)
		);
	}

	if (T <= T0)
	{
		return new VectorValue(
			interpolate(Sn1, Tn1, S0, T0, T)
		);
	}

	return new VectorValue(
		lerp(Sn1, S0, Scalar(T - Tn1) / dT_n1_0)
	);
}

bool VectorTemplate::threshold(const IValue* Vn1, const IValue* V) const
{
	return false;
}

}
