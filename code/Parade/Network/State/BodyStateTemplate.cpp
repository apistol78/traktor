#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.BodyStateTemplate", BodyStateTemplate, IValueTemplate)

void BodyStateTemplate::pack(BitWriter& writer, const IValue* V) const
{
	float T_MATH_ALIGN16 e[4];

	physics::BodyState v = *checked_type_cast< const BodyStateValue* >(V);
	const Transform& T = v.getTransform();

	T.translation().storeAligned(e);
	for (uint32_t i = 0; i < 3; ++i)
		writer.writeUnsigned(32, *(uint32_t*)&e[i]);

	T.rotation().e.storeAligned(e);
	for (uint32_t i = 0; i < 4; ++i)
		packUnit(writer, e[i]);

	v.getLinearVelocity().storeAligned(e);

	writer.writeUnsigned(32, *(uint32_t*)&e[0]);
	writer.writeUnsigned(32, *(uint32_t*)&e[1]);
	writer.writeUnsigned(32, *(uint32_t*)&e[2]);

	v.getAngularVelocity().storeAligned(e);

	writer.writeUnsigned(32, *(uint32_t*)&e[0]);
	writer.writeUnsigned(32, *(uint32_t*)&e[1]);
	writer.writeUnsigned(32, *(uint32_t*)&e[2]);
}

Ref< const IValue > BodyStateTemplate::unpack(BitReader& reader) const
{
	uint32_t u[3];
	float f[4];

	for (uint32_t i = 0; i < 3; ++i)
		u[i] = reader.readUnsigned(32);

	for (uint32_t i = 0; i < 4; ++i)
		f[i] = unpackUnit(reader);

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
			f[3]
		).normalized()
	);

	for (uint32_t i = 0; i < 3; ++i)
		u[i] = reader.readUnsigned(32);

	Vector4 Vl(
		*(float*)&u[0],
		*(float*)&u[1],
		*(float*)&u[2],
		0.0f
	);

	for (uint32_t i = 0; i < 3; ++i)
		u[i] = reader.readUnsigned(32);

	Vector4 Va(
		*(float*)&u[0],
		*(float*)&u[1],
		*(float*)&u[2],
		0.0f
	);

	physics::BodyState S;
	S.setTransform(T);
	S.setLinearVelocity(Vl);
	S.setAngularVelocity(Va);

	return new BodyStateValue(S);
}

Ref< const IValue > BodyStateTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, const IValue* V, float T) const
{
	const Scalar dT(1.0f / 60.0f);

	const physics::BodyState& Sn1 = *checked_type_cast< const BodyStateValue* >(Vn1);
	const physics::BodyState& S0 = *checked_type_cast< const BodyStateValue* >(V0);

	Vector4 Al = (S0.getLinearVelocity() - Sn1.getLinearVelocity()) / Scalar(T0 - Tn1);
	Vector4 Aa = (S0.getAngularVelocity() - Sn1.getAngularVelocity()) / Scalar(T0 - Tn1);

	Vector4 Al_prim = Vector4::zero();
	Vector4 Aa_prim = Vector4::zero();

	if (Vn2)
	{
		const physics::BodyState& Sn2 = *checked_type_cast< const BodyStateValue* >(Vn2);

		Vector4 Al_n2_n1 = (Sn1.getLinearVelocity() - Sn2.getLinearVelocity()) / Scalar(Tn1 - Tn2);
		Vector4 Aa_n2_n1 = (Sn1.getAngularVelocity() - Sn2.getAngularVelocity()) / Scalar(Tn1 - Tn2);

		Al_prim = (Al - Al_n2_n1) / Scalar(T0 - Tn1);
		Aa_prim = (Aa - Aa_n2_n1) / Scalar(T0 - Tn1);
	}

	Vector4 Vl = S0.getLinearVelocity().xyz0();
	Vector4 Va = S0.getAngularVelocity().xyz0();

	Vector4 P = S0.getTransform().translation().xyz1();
	Quaternion R = S0.getTransform().rotation();

	Scalar T_0_c(T - T0);
	while (T_0_c >= dT)
	{
		Al += Al_prim * Scalar(0.5f) * dT * dT;
		Aa += Aa_prim * Scalar(0.5f) * dT * dT;

		Vl += Al * dT;
		Va += Aa * dT;

		P += Vl * dT;
		R = Quaternion::fromAxisAngle(Va * dT) * R;

		T_0_c -= dT;
	}

	if (T_0_c > 0.0f)
	{
		Al += Al_prim * Scalar(0.5f) * T_0_c * T_0_c;
		Aa += Aa_prim * Scalar(0.5f) * T_0_c * T_0_c;

		Vl += Al * T_0_c;
		Va += Aa * T_0_c;

		P += Vl * T_0_c;
		R = Quaternion::fromAxisAngle(Va * T_0_c) * R;
	}

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

		if (ln < 12.0f)
		{
			float k0 = clamp((T - T0) / 0.1f + (ln / 12.0f), 0.0f, 1.0f);
			float k1 = lerp(0.3f, 0.9f, k0);
			S = S.interpolate(Sc, Scalar(k1));
		}
	}

	return new BodyStateValue(S);
}

	}
}
