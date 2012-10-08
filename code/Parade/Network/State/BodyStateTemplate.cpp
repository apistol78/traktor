#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Parade/Network/State/BodyStateValue.h"
#include "Parade/Network/State/BodyStateTemplate.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.BodyStateTemplate", BodyStateTemplate, IValueTemplate)

void BodyStateTemplate::pack(BitWriter& writer, const IValue* V) const
{
	float T_MATH_ALIGN16 e[4 + 4];

	physics::BodyState v = *checked_type_cast< const BodyStateValue* >(V);
	const Transform& T = v.getTransform();

	T.translation().storeAligned(&e[0]);
	T.rotation().e.storeAligned(&e[4]);

	for (uint32_t i = 0; i < 3; ++i)
		writer.writeUnsigned(32, *(uint32_t*)&e[i]);

	for (uint32_t i = 0; i < 4; ++i)
		writer.writeUnsigned(32, *(uint32_t*)&e[4 + i]);

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
	uint32_t u[4 + 4];

	for (uint32_t i = 0; i < 3; ++i)
		u[i] = reader.readUnsigned(32);

	for (uint32_t i = 0; i < 4; ++i)
		u[4 + i] = reader.readUnsigned(32);

	Transform T(
		Vector4(
			*(float*)&u[0],
			*(float*)&u[1],
			*(float*)&u[2],
			1.0f
		),
		Quaternion(
			*(float*)&u[4],
			*(float*)&u[5],
			*(float*)&u[6],
			*(float*)&u[7]
		)
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

Ref< const IValue > BodyStateTemplate::extrapolate(const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const
{
	physics::BodyState Sn1 = *checked_type_cast< const BodyStateValue* >(Vn1);
	physics::BodyState S0 = *checked_type_cast< const BodyStateValue* >(V0);

	const Scalar dT(1.0f / 60.0f);

	Vector4 Al = (S0.getLinearVelocity() - Sn1.getLinearVelocity()) / Scalar(T0 - Tn1);
	Vector4 Aa = (S0.getAngularVelocity() - Sn1.getAngularVelocity()) / Scalar(T0 - Tn1);

	Vector4 Vl = S0.getLinearVelocity().xyz0();
	Vector4 Va = S0.getAngularVelocity().xyz0();

	Vector4 P = S0.getTransform().translation().xyz1();
	Quaternion R = S0.getTransform().rotation();

	Scalar T_0_c(T - T0);
	while (T_0_c >= dT)
	{
		Vl += Al * dT;
		Va += Aa * dT;

		P += Vl * dT;
		R = Quaternion::fromAxisAngle(Va * dT) * R;

		T_0_c -= dT;
	}

	if (T_0_c > 0.0f)
	{
		Vl += Al * T_0_c;
		Va += Aa * T_0_c;

		P += Vl * T_0_c;
		R = Quaternion::fromAxisAngle(Va * T_0_c) * R;
	}

	physics::BodyState S;
	S.setTransform(Transform(P, R.normalized()));
	S.setLinearVelocity(Vl);
	S.setAngularVelocity(Va);

	return new BodyStateValue(S);
}

Ref< const IValue > BodyStateTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const
{
	physics::BodyState Sn2 = *checked_type_cast< const BodyStateValue* >(Vn2);
	physics::BodyState Sn1 = *checked_type_cast< const BodyStateValue* >(Vn1);
	physics::BodyState S0 = *checked_type_cast< const BodyStateValue* >(V0);

	const Scalar dT(1.0f / 60.0f);

	Vector4 Al_n2_n1 = (Sn1.getLinearVelocity() - Sn2.getLinearVelocity()) / Scalar(Tn1 - Tn2);
	Vector4 Aa_n2_n1 = (Sn1.getAngularVelocity() - Sn2.getAngularVelocity()) / Scalar(Tn1 - Tn2);

	Vector4 Al_n1_0 = (S0.getLinearVelocity() - Sn1.getLinearVelocity()) / Scalar(T0 - Tn1);
	Vector4 Aa_n1_0 = (S0.getAngularVelocity() - Sn1.getAngularVelocity()) / Scalar(T0 - Tn1);

	Vector4 Al_prim = (Al_n1_0 - Al_n2_n1) / Scalar(T0 - Tn1);
	Vector4 Aa_prim = (Aa_n1_0 - Aa_n2_n1) / Scalar(T0 - Tn1);

	Vector4 Al = Al_n1_0;
	Vector4 Aa = Aa_n1_0;

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

	return new BodyStateValue(S);
}

	}
}
