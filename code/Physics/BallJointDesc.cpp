#include "Physics/BallJointDesc.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.BallJointDesc", BallJointDesc, JointDesc)

BallJointDesc::BallJointDesc()
:	m_anchor(0.0f, 0.0f, 0.0f, 1.0f)
{
}

void BallJointDesc::setAnchor(const Vector4& anchor)
{
	m_anchor = anchor;
}

const Vector4& BallJointDesc::getAnchor() const
{
	return m_anchor;
}

bool BallJointDesc::serialize(Serializer& s)
{
	return s >> Member< Vector4 >(L"anchor", m_anchor);
}

	}
}
