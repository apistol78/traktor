#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Model/Pose.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.Pose", 0, Pose, ISerializable)

void Pose::setJointTransform(uint32_t jointId, const Transform& jointTransform)
{
	if (jointId >= m_jointTransforms.size())
		m_jointTransforms.resize(jointId + 1, Transform::identity());
	m_jointTransforms[jointId] = jointTransform;
}

const Transform& Pose::getJointTransform(uint32_t jointId) const
{
	return m_jointTransforms[jointId];
}

void Pose::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Transform, MemberComposite< Transform > >(L"jointTransforms", m_jointTransforms);
}

	}
}
