/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Serialization/PackedUnitVector.h"
#include "Jungle/State/TransformTemplate.h"
#include "Jungle/State/TransformValue.h"

namespace traktor::jungle
{
	namespace
	{

float safeDeltaTime(float v)
{
	const float av = std::abs(v);
	if (av < 1.0f/60.0f)
		return 1.0f/60.0f * sign(v);
	else if (av > 0.5f)
		return 1.0f * sign(v);
	else
		return v;
}

Transform interpolate(const Transform& t0, float T0, const Transform& t1, float T1, float T)
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.jungle.TransformTemplate", TransformTemplate, IValueTemplate)

TransformTemplate::TransformTemplate(const std::wstring& tag)
:	m_tag(tag)
{
}

const TypeInfo& TransformTemplate::getValueType() const
{
	return type_of< TransformValue >();
}

uint32_t TransformTemplate::getMaxPackedDataSize() const
{
	return 3 * (13 + 11) + 16 + (4 + 11);
}

void TransformTemplate::pack(BitWriter& writer, const IValue* V) const
{
	const Transform v = *mandatory_non_null_type_cast< const TransformValue* >(V);
	float T_MATH_ALIGN16 e[4];

	// 3 * (13+11)
	v.translation().storeAligned(e);
	for (uint32_t i = 0; i < 3; ++i)
		writer.writeSigned(13 + 11, GenericFixedPoint< 13, 11 >(e[i]).raw());

	// 16 + (4+11)
	Vector4 R = v.rotation().toAxisAngle();
	const Scalar a = R.length();
	if (abs(a) > FUZZY_EPSILON)
		R /= a;

	writer.writeUnsigned(16, PackedUnitVector(R).raw());
	writer.writeSigned(4 + 11, GenericFixedPoint< 4, 11 >(a).raw());
}

Ref< const IValue > TransformTemplate::unpack(BitReader& reader) const
{
	float T_MATH_ALIGN16 f[4];
	uint16_t u;

	for (uint32_t i = 0; i < 3; ++i)
	{
		f[i] = GenericFixedPoint< 13, 11 >(reader.readSigned(13 + 11));
		T_ASSERT(!isNanOrInfinite(f[i]));
	}
	f[3] = 1.0f;

	u = reader.readUnsigned(16);
	const Vector4 R = PackedUnitVector(u).unpack();
	const float Ra = GenericFixedPoint< 4, 11 >(reader.readSigned(4 + 11));

	Transform v(
		Vector4::loadAligned(f),
		(abs(Ra) > FUZZY_EPSILON && R.length() > FUZZY_EPSILON) ?
			Quaternion::fromAxisAngle(R, Ra).normalized() :
			Quaternion::identity()
	);

	return new TransformValue(v);
}

Ref< const IValue > TransformTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const
{
	const Transform Sn2 = *mandatory_non_null_type_cast< const TransformValue* >(Vn2);
	const Transform Sn1 = *mandatory_non_null_type_cast< const TransformValue* >(Vn1);
	const Transform S0 = *mandatory_non_null_type_cast< const TransformValue* >(V0);

	Scalar dT_0(safeDeltaTime(T - T0));
	Scalar dT_n1_0(safeDeltaTime(T0 - Tn1));
	Scalar dT_n2_n1(safeDeltaTime(Tn1 - Tn2));

	if (T <= Tn2)
		return Vn2;

	if (T <= Tn1)
	{
		return new TransformValue(
			interpolate(Sn2, Tn2, Sn1, Tn1, T)
		);
	}

	if (T <= T0)
	{
		return new TransformValue(
			interpolate(Sn1, Tn1, S0, T0, T)
		);
	}

	return new TransformValue(
		lerp(Sn1, S0, Scalar(T - Tn1) / dT_n1_0)
	);
}

bool TransformTemplate::threshold(const IValue* Vn1, const IValue* V) const
{
	return false;
}

}
