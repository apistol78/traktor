#include <algorithm>
#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Net/Replication/Pack.h"
#include "Net/Replication/State/BodyStateValue.h"
#include "Net/Replication/State/BodyStateTemplate.h"

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

physics::BodyState interpolate(const physics::BodyState& bs0, float T0, const physics::BodyState& bs1, float T1, float T)
{
	Scalar dT_0_1 = Scalar(safeDeltaTime(T1 - T0));

#if 0

	physics::BodyState state;

	state.setTransform(lerp(bs0.getTransform(), bs1.getTransform(), Scalar(T - T0) / dT_0_1));
	state.setLinearVelocity(
		(bs1.getTransform().translation() - bs0.getTransform().translation()) / dT_0_1
	);

	Quaternion Qv0 = bs0.getTransform().rotation();
	Quaternion Qv1 = Qv0.nearest(bs1.getTransform().rotation());
	Quaternion Qdiff = Qv1 * Qv0.inverse();

	Vector4 Vdiff = Qdiff.toAxisAngle();
	Scalar angleDiff = Vdiff.length();

	if (angleDiff > FUZZY_EPSILON)
		state.setAngularVelocity(Vdiff / dT_0_1);
	else
		state.setAngularVelocity(Vector4::zero());

	return state;

#else

	return bs0.interpolate(bs1, Scalar(T - T0) / dT_0_1);

#endif
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.BodyStateTemplate", BodyStateTemplate, IValueTemplate)

BodyStateTemplate::BodyStateTemplate(float linearError, float angularError)
:	m_linearError(linearError)
,	m_angularError(angularError)
{
}

const TypeInfo& BodyStateTemplate::getValueType() const
{
	return type_of< BodyStateValue >();
}

void BodyStateTemplate::pack(BitWriter& writer, const IValue* V) const
{
	physics::BodyState v = *checked_type_cast< const BodyStateValue* >(V);
	const Transform& T = v.getTransform();
	float T_MATH_ALIGN16 e[4];

	// 3 * (13+11)
	T.translation().storeAligned(e);
	for (uint32_t i = 0; i < 3; ++i)
		writer.writeSigned(13+11, GenericFixedPoint< 13, 11 >(e[i]).raw());

	// 16 + (4+11)
	Vector4 R = T.rotation().toAxisAngle();
	
	float a = R.length();
	if (abs(a) > FUZZY_EPSILON)
		R /= Scalar(a);

	writer.writeUnsigned(16, PackedUnitVector(R).raw());
	writer.writeSigned(4+11, GenericFixedPoint< 4, 11 >(a).raw());

	// 16 + (7+8)
	{
		Vector4 linearVelocity = v.getLinearVelocity().xyz0();
		Scalar ln = linearVelocity.length();

		if (ln > FUZZY_EPSILON)
			linearVelocity /= ln;

		writer.writeUnsigned(16, PackedUnitVector(linearVelocity).raw());
		writer.writeSigned(7+8, GenericFixedPoint< 7, 8 >(ln).raw());
	}

	// 16 + (5+8)
	{
		Vector4 angularVelocity = v.getAngularVelocity().xyz0();
		Scalar ln = angularVelocity.length();

		if (ln > FUZZY_EPSILON)
			angularVelocity /= ln;

		writer.writeUnsigned(16, PackedUnitVector(angularVelocity).raw());
		writer.writeSigned(5+8, GenericFixedPoint< 5, 8 >(ln).raw());
	}
}

Ref< const IValue > BodyStateTemplate::unpack(BitReader& reader) const
{
	Vector4 linearVelocity, angularVelocity;
	Transform T;
	float T_MATH_ALIGN16 f[4];
	uint16_t u;

	for (uint32_t i = 0; i < 3; ++i)
	{
		f[i] = GenericFixedPoint< 13, 11 >(reader.readSigned(13+11));
		T_FATAL_ASSERT(!isNanOrInfinite(f[i]));
	}

	u = reader.readUnsigned(16);
	Vector4 R = PackedUnitVector(u).unpack();
	float Ra = GenericFixedPoint< 4, 11 >(reader.readSigned(4+11));

	T = Transform(
		Vector4(f[0], f[1], f[2], 1.0f),
		(abs(Ra) > FUZZY_EPSILON && R.length() > FUZZY_EPSILON) ? 
			Quaternion::fromAxisAngle(R, Ra).normalized() :
			Quaternion::identity()
	);

	u = reader.readUnsigned(16);
	linearVelocity = PackedUnitVector(u).unpack();
	linearVelocity *= Scalar(GenericFixedPoint< 7, 8 >(reader.readSigned(7+8)));

	u = reader.readUnsigned(16);
	angularVelocity = PackedUnitVector(u).unpack();
	angularVelocity *= Scalar(GenericFixedPoint< 5, 8 >(reader.readSigned(5+8)));

	physics::BodyState S;
	S.setTransform(T);
	S.setLinearVelocity(linearVelocity);
	S.setAngularVelocity(angularVelocity);

	return new BodyStateValue(S);
}

Ref< const IValue > BodyStateTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const
{
	const physics::BodyState& Sn2 = *checked_type_cast< const BodyStateValue* >(Vn2);
	const physics::BodyState& Sn1 = *checked_type_cast< const BodyStateValue* >(Vn1);
	const physics::BodyState& S0 = *checked_type_cast< const BodyStateValue* >(V0);

	Scalar dT_0(safeDeltaTime(T - T0));
	Scalar dT_n1_0(safeDeltaTime(T0 - Tn1));
	Scalar dT_n2_n1(safeDeltaTime(Tn1 - Tn2));

	if (T <= Tn2)
		return Vn2;

	if (T <= Tn1)
	{
		return new BodyStateValue(
			interpolate(Sn2, Tn2, Sn1, Tn1, T)
		);
	}

	if (T <= T0)
	{
		return new BodyStateValue(
			interpolate(Sn1, Tn1, S0, T0, T)
		);
	}

	Vector4 Vl = S0.getLinearVelocity().xyz0();
	Vector4 Va = S0.getAngularVelocity();

	Vector4 P = S0.getTransform().translation().xyz1();
	Quaternion R = S0.getTransform().rotation();

	P = P + (Vl * dT_0);
	R = Quaternion::fromAxisAngle(S0.getAngularVelocity() * dT_0) * R;

	physics::BodyState Sf;
	Sf.setTransform(Transform(P, R.normalized()));
	Sf.setLinearVelocity(Vl);
	Sf.setAngularVelocity(Va);

	return new BodyStateValue(Sf);
}

bool BodyStateTemplate::threshold(const IValue* Vn1, const IValue* V) const
{
	return false;
}

	}
}
