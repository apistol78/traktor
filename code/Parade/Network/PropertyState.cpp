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
:	m_offset(0)
{
}

void PropertyState::packFloat(float f)
{
	m_data.push_back(f);
}

void PropertyState::packVector3(const Vector4& v)
{
	uint32_t offset = m_data.size(); m_data.resize(offset + 3);
	m_data[offset++] = v.x();
	m_data[offset++] = v.y();
	m_data[offset  ] = v.z();
}

void PropertyState::packVector4(const Vector4& v)
{
	uint32_t offset = m_data.size(); m_data.resize(offset + 4);
	v.storeUnaligned(&m_data[offset]);
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
	m_offset = 0;
}

float PropertyState::unpackFloat() const
{
	const float* data = &m_data[m_offset++];
	return *data;
}

Vector4 PropertyState::unpackVector3(float w) const
{
	const float* data = &m_data[m_offset]; m_offset += 3;
	return Vector4(data[0], data[1], data[2], w);
}

Vector4 PropertyState::unpackVector4() const
{
	const float* data = &m_data[m_offset]; m_offset += 4;
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
	return (targetPropertyState->m_data.size() == m_data.size());
}

Ref< IReplicatableState > PropertyState::extrapolate(const IReplicatableState* targetState, float T) const
{
	const PropertyState* targetPropertyState = checked_type_cast< const PropertyState*, false >(targetState);
	T_ASSERT (targetPropertyState->m_data.size() == m_data.size());

	Ref< PropertyState > extrapolatedState = new PropertyState();
	extrapolatedState->m_data.resize(m_data.size());

	for (uint32_t i = 0; i < m_data.size(); ++i)
		extrapolatedState->m_data[i] = lerp(m_data[i], targetPropertyState->m_data[i], T);

	return extrapolatedState;
}

bool PropertyState::serialize(ISerializer& s)
{
	return s >> MemberStlVector< float >(L"data", m_data);
}

	}
}
