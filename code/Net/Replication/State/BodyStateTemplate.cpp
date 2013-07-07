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
#include "Net/Replication/State/Config.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

float errorV4(const Vector4& Vl, const Vector4& Vr)
{
	return (Vl - Vr).length();
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

physics::BodyState interpolate(const physics::BodyState& bs0, float T0, const physics::BodyState& bs1, float T1, float T)
{
#if 0

	physics::BodyState state;

	state.setTransform(lerp(bs0.getTransform(), bs1.getTransform(), Scalar( (T - T0) / (T1 - T0) )));

	state.setLinearVelocity(
		(bs1.getTransform().translation() - bs0.getTransform().translation()) / Scalar(T1 - T0)
	);

	Quaternion Qv0 = bs0.getTransform().rotation();
	Quaternion Qv1 = Qv0.nearest(bs1.getTransform().rotation());
	Quaternion Qdiff = Qv1 * Qv0.inverse();

	Vector4 Vdiff = Qdiff.toAxisAngle();
	Scalar angleDiff = Vdiff.length();

	if (angleDiff > FUZZY_EPSILON)
		state.setAngularVelocity(Vdiff / Scalar(T1 - T0));
	else
		state.setAngularVelocity(Vector4::zero());

	return state;

#else

	return bs0.interpolate(bs1, Scalar( (T - T0) / (T1 - T0) ));

#endif
}

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

	}
}
