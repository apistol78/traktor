#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Parade/Network/State/BodyStateValue.h"
#include "Parade/Network/State/BodyStateTemplate.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

const Scalar c_maxRubberBandDistance(20.0f);
const float c_maxRubberBandTime(0.1f);
const float c_rubberBandStrengthNear(0.3f);
const float c_rubberBandStrengthFar(0.9f);

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

Vector4 clampV4(const Vector4& v, const Vector4& minV, const Vector4& maxV)
{
	return max(min(v, maxV), minV);
}

float safeSqrt(float v)
{
	if (v > 0.0f)
		return std::sqrt(v);
	else
		return 0.0f;
}

float safeDenom(float v)
{
	if (abs(v) < FUZZY_EPSILON)
		return FUZZY_EPSILON * sign(v);
	else
		return v;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.BodyStateTemplate", BodyStateTemplate, IValueTemplate)

void BodyStateTemplate::pack(BitWriter& writer, const IValue* V) const
{
	float e[4];

	physics::BodyState v = *checked_type_cast< const BodyStateValue* >(V);
	const Transform& T = v.getTransform();

	// 3 * 32
	T.translation().storeUnaligned(e);
	for (uint32_t i = 0; i < 3; ++i)
		writer.writeUnsigned(32, *(uint32_t*)&e[i]);

	// 3 * 16
	T.rotation().e.storeUnaligned(e);
	for (uint32_t i = 0; i < 3; ++i)
		packUnit(writer, e[i]);

	// 1
	writer.writeBit(e[3] < 0.0f);

	{
		Vector4 linearVelocity = v.getLinearVelocity().xyz0();
		float ln = linearVelocity.length();
		if (abs(ln) > 1.0f/512.0f)
		{
			// 1
			writer.writeBit(true);

			linearVelocity /= Scalar(ln);

			// 2 * 16
			packUnit(writer, linearVelocity.x());
			packUnit(writer, linearVelocity.y());

			// 1
			writer.writeBit(linearVelocity.z() < 0.0f);

			// 16
			uint32_t uln = std::min< uint32_t >(uint32_t(ln * 512.0f), 65535);
			writer.writeUnsigned(16, uln);
		}
		else
			// 1
			writer.writeBit(false);
	}

	{
		Vector4 angularVelocity = v.getAngularVelocity().xyz0();
		float ln = angularVelocity.length();
		if (abs(ln) > 1.0f/512.0f)
		{
			// 1
			writer.writeBit(true);

			angularVelocity /= Scalar(ln);

			// 2 * 16
			packUnit(writer, angularVelocity.x());
			packUnit(writer, angularVelocity.y());

			// 1
			writer.writeBit(angularVelocity.z() < 0.0f);

			// 16
			uint32_t uln = std::min< uint32_t >(uint32_t(ln * 512.0f), 65535);
			writer.writeUnsigned(16, uln);
		}
		else
			// 1
			writer.writeBit(false);
	}
}

Ref< const IValue > BodyStateTemplate::unpack(BitReader& reader) const
{
	uint32_t u[3];
	float f[4];

	for (uint32_t i = 0; i < 3; ++i)
		u[i] = reader.readUnsigned(32);

	for (uint32_t i = 0; i < 3; ++i)
		f[i] = unpackUnit(reader);

	float sign = reader.readBit() ? -1.0f : 1.0f;

	Transform T(
		Vector4(
			*(float*)&u[0],
			*(float*)&u[1],
			*(float*)&u[2],
			1.0f
		),
		Quaternion(
			f[0],
			f[1],
			f[2],
			safeSqrt(1.0f - f[0] * f[0] - f[1] * f[1] - f[2] * f[2]) * sign
		).normalized()
	);

	Vector4 linearVelocity = Vector4::zero();
	if (reader.readBit())
	{
		float x = unpackUnit(reader);
		float y = unpackUnit(reader);
		float z = safeSqrt(1.0f - x * x - y * y);

		if (reader.readBit())
			z = -z;
		
		linearVelocity = Vector4(x, y, z, 0.0f);

		Scalar ln1 = linearVelocity.length();
		if (ln1 > FUZZY_EPSILON)
			linearVelocity /= ln1;

		float ln2 = reader.readUnsigned(16) / 512.0f;
		linearVelocity *= Scalar(ln2);
	}

	Vector4 angularVelocity = Vector4::zero();
	if (reader.readBit())
	{
		float x = unpackUnit(reader);
		float y = unpackUnit(reader);
		float z = safeSqrt(1.0f - x * x - y * y);

		if (reader.readBit())
			z = -z;

		angularVelocity = Vector4(x, y, z, 0.0f);

		Scalar ln1 = angularVelocity.length();
		if (ln1 > FUZZY_EPSILON)
			angularVelocity /= ln1;

		float ln2 = reader.readUnsigned(16) / 512.0f;
		angularVelocity *= Scalar(ln2);
	}

	physics::BodyState S;
	S.setTransform(T);
	S.setLinearVelocity(linearVelocity);
	S.setAngularVelocity(angularVelocity);

	return new BodyStateValue(S);
}

Ref< const IValue > BodyStateTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, const IValue* V, float T) const
{
	const Vector4 c_linearAccThreshold(1.0f, 1.0f, 1.0f, 0.0f);
	const Vector4 c_angularAccThreshold(0.3f, 0.3f, 0.3f, 0.0f);

	Scalar dT_0(safeDenom(T - T0));
	Scalar dT_n1_0(safeDenom(T0 - Tn1));
	Scalar dT_n2_n1(safeDenom(Tn1 - Tn2));

	const physics::BodyState& Sn1 = *checked_type_cast< const BodyStateValue* >(Vn1);
	const physics::BodyState& S0 = *checked_type_cast< const BodyStateValue* >(V0);

	Vector4 Al = (S0.getLinearVelocity() - Sn1.getLinearVelocity()) / dT_n1_0;
	Vector4 Aa = (S0.getAngularVelocity() - Sn1.getAngularVelocity()) / dT_n1_0;

	Vector4 Al_prim = Vector4::zero();
	Vector4 Aa_prim = Vector4::zero();

	if (Vn2)
	{
		const physics::BodyState& Sn2 = *checked_type_cast< const BodyStateValue* >(Vn2);

		Vector4 Al_n2_n1 = (Sn1.getLinearVelocity() - Sn2.getLinearVelocity()) / dT_n2_n1;
		Vector4 Aa_n2_n1 = (Sn1.getAngularVelocity() - Sn2.getAngularVelocity()) / dT_n2_n1;

		Al_prim = (Al - Al_n2_n1) / Scalar(dT_n1_0);
		Aa_prim = (Aa - Aa_n2_n1) / Scalar(dT_n1_0);

		Al_prim = clampV4(Al_prim, -c_linearAccThreshold, c_linearAccThreshold);
		Aa_prim = clampV4(Aa_prim, -c_angularAccThreshold, c_angularAccThreshold);
	}

	Vector4 Vl = S0.getLinearVelocity().xyz0();
	Vector4 Va = S0.getAngularVelocity().xyz0();

	Vector4 P = S0.getTransform().translation().xyz1();
	Quaternion R = S0.getTransform().rotation();

	Al += Al_prim * Scalar(0.5f) * dT_0 * dT_0;
	Aa += Aa_prim * Scalar(0.5f) * dT_0 * dT_0;

	Vl += Al * dT_0;
	Va += Aa * dT_0;

	P += Vl * dT_0;
	R = Quaternion::fromAxisAngle(Va * dT_0) * R;

	physics::BodyState S;
	S.setTransform(Transform(P, R.normalized()));
	S.setLinearVelocity(Vl);
	S.setAngularVelocity(Va);

	// If current simulated state is known then blend into it if last known
	// state is becoming too old or extrapolated too far away.
	if (V)
	{
		const physics::BodyState& Sc = *checked_type_cast< const BodyStateValue* >(V);

		Vector4 Pc = Sc.getTransform().translation();
		Scalar ln = (P - Pc).length();

		if (ln < c_maxRubberBandDistance)
		{
			float k0 = dT_0 / c_maxRubberBandTime;
			float k1 = ln / c_maxRubberBandDistance;
			float k2 = clamp(max(k0, k1), 0.0f, 1.0f);
			float k3 = lerp(c_rubberBandStrengthNear, c_rubberBandStrengthFar, k2);
			S = S.interpolate(Sc, Scalar(k3));
		}
	}

	return new BodyStateValue(S);
}

	}
}
