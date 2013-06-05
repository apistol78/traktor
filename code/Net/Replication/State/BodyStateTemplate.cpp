#include <algorithm>
#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Net/Replication/Pack.h"
#include "Net/Replication/State/BodyStateValue.h"
#include "Net/Replication/State/BodyStateTemplate.h"
#include "Net/Replication/State/Config.h"

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

Vector4 clampV4(const Vector4& v, const Vector4& minV, const Vector4& maxV)
{
	return max(min(v, maxV), minV);
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.BodyStateTemplate", BodyStateTemplate, IValueTemplate)

BodyStateTemplate::BodyStateTemplate(float linearError, float angularError)
:	m_linearError(linearError)
,	m_angularError(angularError)
{
}

void BodyStateTemplate::pack(BitWriter& writer, const IValue* V) const
{
	physics::BodyState v = *checked_type_cast< const BodyStateValue* >(V);
	const Transform& T = v.getTransform();

#if 0

	float e[4];

	T.translation().storeUnaligned(e);
	for (uint32_t i = 0; i < 3; ++i)
		writer.writeUnsigned(32, *(uint32_t*)&e[i]);

	T.rotation().toAxisAngle().storeUnaligned(e);
	for (uint32_t i = 0; i < 3; ++i)
		writer.writeUnsigned(32, *(uint32_t*)&e[i]);

	v.getLinearVelocity().storeUnaligned(e);
	for (uint32_t i = 0; i < 3; ++i)
		writer.writeUnsigned(32, *(uint32_t*)&e[i]);

	v.getAngularVelocity().storeUnaligned(e);
	for (uint32_t i = 0; i < 3; ++i)
		writer.writeUnsigned(32, *(uint32_t*)&e[i]);

#else

	float e[4];
	uint8_t u[3];

	// 3 * 32
	T.translation().storeUnaligned(e);
	for (uint32_t i = 0; i < 3; ++i)
		writer.writeUnsigned(32, *(uint32_t*)&e[i]);

	// 3 * 8 + 32
	Vector4 R = T.rotation().toAxisAngle();
	float a = R.length();
	if (abs(a) > FUZZY_EPSILON)
		R /= Scalar(a);

	packUnit(R, u);
	writer.writeUnsigned(8, u[0]);
	writer.writeUnsigned(8, u[1]);
	writer.writeUnsigned(8, u[2]);
	writer.writeUnsigned(32, *(uint32_t*)&a);

	{
		Vector4 linearVelocity = v.getLinearVelocity().xyz0();
		float ln = linearVelocity.length();
		if (abs(ln) > 1.0f/512.0f)
		{
			// 1
			writer.writeBit(true);

			linearVelocity /= Scalar(ln);

			// 3 * 8
			packUnit(linearVelocity, u);
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

	{
		Vector4 angularVelocity = v.getAngularVelocity().xyz0();
		float ln = angularVelocity.length();
		if (abs(ln) > 1.0f/512.0f)
		{
			// 1
			writer.writeBit(true);

			angularVelocity /= Scalar(ln);

			// 3 * 8
			packUnit(angularVelocity, u);
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

#endif
}

Ref< const IValue > BodyStateTemplate::unpack(BitReader& reader) const
{
	Vector4 linearVelocity, angularVelocity;
	Transform T;

#if 0

	uint32_t ut[3], ur[3];
	uint32_t ulv[3], uav[3];

	for (uint32_t i = 0; i < 3; ++i)
		ut[i] = reader.readUnsigned(32);
	for (uint32_t i = 0; i < 3; ++i)
		ur[i] = reader.readUnsigned(32);
	for (uint32_t i = 0; i < 3; ++i)
		ulv[i] = reader.readUnsigned(32);
	for (uint32_t i = 0; i < 3; ++i)
		uav[i] = reader.readUnsigned(32);

	T = Transform(
		Vector4(
			*(float*)&ut[0],
			*(float*)&ut[1],
			*(float*)&ut[2],
			1.0f
		),
		Quaternion::fromAxisAngle(
			Vector4(
				*(float*)&ur[0],
				*(float*)&ur[1],
				*(float*)&ur[2],
				0.0f
			)
		).normalized()
	);

	linearVelocity = Vector4(
		*(float*)&ulv[0],
		*(float*)&ulv[1],
		*(float*)&ulv[2],
		0.0f
	);
	angularVelocity = Vector4(
		*(float*)&uav[0],
		*(float*)&uav[1],
		*(float*)&uav[2],
		0.0f
	);

#else

	uint32_t uf[3];
	uint8_t u[3];

	for (uint32_t i = 0; i < 3; ++i)
		uf[i] = reader.readUnsigned(32);

	u[0] = reader.readUnsigned(8);
	u[1] = reader.readUnsigned(8);
	u[2] = reader.readUnsigned(8);
	Vector4 R = unpackUnit(u);
	uint32_t ua = reader.readUnsigned(32);

	T = Transform(
		Vector4(
			*(float*)&uf[0],
			*(float*)&uf[1],
			*(float*)&uf[2],
			1.0f
		),
		Quaternion::fromAxisAngle(R, *(float*)&ua).normalized()
	);

	linearVelocity = Vector4::zero();
	if (reader.readBit())
	{
		uint8_t u[3];
		u[0] = reader.readUnsigned(8);
		u[1] = reader.readUnsigned(8);
		u[2] = reader.readUnsigned(8);
		linearVelocity = unpackUnit(u);

		float ln2 = reader.readUnsigned(16) / 512.0f;
		linearVelocity *= Scalar(ln2);
	}

	angularVelocity = Vector4::zero();
	if (reader.readBit())
	{
		uint8_t u[3];
		u[0] = reader.readUnsigned(8);
		u[1] = reader.readUnsigned(8);
		u[2] = reader.readUnsigned(8);
		angularVelocity = unpackUnit(u);

		float ln2 = reader.readUnsigned(16) / 512.0f;
		angularVelocity *= Scalar(ln2);
	}

#endif

	physics::BodyState S;
	S.setTransform(T);
	S.setLinearVelocity(linearVelocity);
	S.setAngularVelocity(angularVelocity);

	return new BodyStateValue(S);
}

float BodyStateTemplate::error(const IValue* Vl, const IValue* Vr) const
{
	const physics::BodyState& Sl = *checked_type_cast< const BodyStateValue* >(Vl);
	const physics::BodyState& Sr = *checked_type_cast< const BodyStateValue* >(Vr);

	float errors[] =
	{
		errorV4(Sl.getTransform().translation(), Sr.getTransform().translation()) * m_linearError,
		errorV4(Sl.getTransform().rotation().e, Sr.getTransform().rotation().e) * m_angularError,
		errorV4(Sl.getLinearVelocity(), Sr.getLinearVelocity()) * m_linearError,
		errorV4(Sl.getAngularVelocity(), Sr.getAngularVelocity()) * m_angularError
	};

	return *std::max_element(&errors[0], &errors[3]);
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
			Sn2.interpolate(Sn1, Scalar(T - Tn2) / dT_n2_n1)
		);
	}

	if (T <= T0)
	{
		return new BodyStateValue(
			Sn1.interpolate(S0, Scalar(T - Tn1) / dT_n1_0)
		);
	}

	Vector4 Vl = S0.getLinearVelocity().xyz0();
	Vector4 Vl_n1 = Sn1.getLinearVelocity().xyz0();
	Quaternion Va = Quaternion::fromAxisAngle(S0.getAngularVelocity());
	Quaternion Va_n1 = Quaternion::fromAxisAngle(Sn1.getAngularVelocity());

	Vector4 P = S0.getTransform().translation().xyz1();
	Quaternion R = S0.getTransform().rotation();

	P = (Vl * dT_0) + P;
	R = Quaternion::fromAxisAngle(S0.getAngularVelocity() * dT_0) * R;

	Vl = lerp(Vl_n1, Vl, Scalar(T - Tn1) / dT_n1_0);

	Quaternion Qdiff = Va * Va_n1.inverse();
	Vector4 Vdiff = Qdiff.toAxisAngle();
	Scalar angleDiff = Vdiff.length();
	if (abs(angleDiff) > FUZZY_EPSILON)
	{
		Va = Quaternion::fromAxisAngle(
			Vdiff / angleDiff,
			angleDiff * Scalar(T - Tn1) / dT_n1_0
		) * Va_n1;
	}

	physics::BodyState Sf;
	Sf.setTransform(Transform(P, R.normalized()));
	Sf.setLinearVelocity(Vl);
	Sf.setAngularVelocity(Va.toAxisAngle());

	return new BodyStateValue(Sf);
}

	}
}
