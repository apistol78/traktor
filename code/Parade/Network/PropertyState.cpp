#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Parade/Network/PropertyState.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.parade.PropertyState", 0, PropertyState, IReplicatableState)

PropertyState::PropertyState()
:	m_offsetU8(0)
,	m_offsetF32(0)
{
}

void PropertyState::packByte(uint8_t b)
{
	m_u8.push_back(b);
}

void PropertyState::packLowPrecision(float f, float min, float max)
{
	packByte(uint8_t(clamp((f - min) / (max - min), 0.0f, 1.0f) * 255));
}

void PropertyState::packFloat(float f)
{
	m_f32.push_back(f);
}

void PropertyState::packVector3(const Vector4& v)
{
	uint32_t offset = m_f32.size(); m_f32.resize(offset + 3);
	m_f32[offset++] = v.x();
	m_f32[offset++] = v.y();
	m_f32[offset  ] = v.z();
}

void PropertyState::packVector4(const Vector4& v)
{
	uint32_t offset = m_f32.size(); m_f32.resize(offset + 4);
	v.storeUnaligned(&m_f32[offset]);
}

void PropertyState::packQuaternion(const Quaternion& q)
{
	packVector4(q.e);
}

void PropertyState::packTransform(const Transform& t)
{
	packVector3(t.translation());
	packQuaternion(t.rotation());
}

void PropertyState::unpackBegin()
{
	m_offsetU8 = 0;
	m_offsetF32 = 0;
}

uint8_t PropertyState::unpackByte() const
{
	return m_u8[m_offsetU8++];
}

float PropertyState::unpackLowPrecision(float min, float max) const
{
	uint8_t b = unpackByte();
	return (b / 255.0f) * (max - min) + min;
}

float PropertyState::unpackFloat() const
{
	return m_f32[m_offsetF32++];
}

Vector4 PropertyState::unpackVector3(float w) const
{
	const float* data = &m_f32[m_offsetF32]; m_offsetF32 += 3;
	return Vector4(data[0], data[1], data[2], w);
}

Vector4 PropertyState::unpackVector4() const
{
	const float* data = &m_f32[m_offsetF32]; m_offsetF32 += 4;
	return Vector4::loadUnaligned(data);
}

Quaternion PropertyState::unpackQuaternion() const
{
	Quaternion q; q.e = unpackVector4();
	return q.normalized();
}

Transform PropertyState::unpackTransform() const
{
	Vector4 t = unpackVector3(0.0f);
	Quaternion r = unpackQuaternion();
	return Transform(t, r);
}

bool PropertyState::verify(const IReplicatableState* targetState) const
{
	const PropertyState* targetPropertyState = checked_type_cast< const PropertyState*, false >(targetState);
	return
		targetPropertyState->m_u8.size() == m_u8.size() &&
		targetPropertyState->m_f32.size() == m_f32.size();
}

Ref< IReplicatableState > PropertyState::extrapolate(const IReplicatableState* targetState, float T) const
{
	const PropertyState* targetPropertyState = checked_type_cast< const PropertyState*, false >(targetState);
	T_ASSERT (targetPropertyState->m_u8.size() == m_u8.size());
	T_ASSERT (targetPropertyState->m_f32.size() == m_f32.size());

	Ref< PropertyState > extrapolatedState = new PropertyState();
	extrapolatedState->m_u8.resize(m_u8.size());
	extrapolatedState->m_f32.resize(m_f32.size());

	for (uint32_t i = 0; i < m_u8.size(); ++i)
		extrapolatedState->m_u8[i] = uint8_t(lerp(float(m_u8[i]), float(targetPropertyState->m_u8[i]), T));

	for (uint32_t i = 0; i < m_f32.size(); ++i)
		extrapolatedState->m_f32[i] = lerp(m_f32[i], targetPropertyState->m_f32[i], T);

	return extrapolatedState;
}

bool PropertyState::serialize(ISerializer& s)
{
	s >> MemberStlVector< uint8_t >(L"u8", m_u8);
	s >> MemberStlVector< float >(L"f32", m_f32);
	return true;
}

	}
}
