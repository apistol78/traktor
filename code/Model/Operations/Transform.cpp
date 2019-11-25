#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Model/Model.h"
#include "Model/Pose.h"
#include "Model/Operations/Transform.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Transform", Transform, IModelOperation)

Transform::Transform(const Matrix44& tf)
:	m_transform(tf)
{
}

bool Transform::apply(Model& model) const
{
	AlignedVector< Vector4 > positions = model.getPositions();
	for (auto& position : positions)
		position = m_transform * position;
	model.setPositions(positions);

	AlignedVector< Vector4 > normals = model.getNormals();
	for (auto& normal : normals)
		normal = (m_transform * normal).normalized();
	model.setNormals(normals);

	AlignedVector< Joint > joints = model.getJoints();
	for (auto& joint : joints)
	{
		traktor::Transform jt0 = joint.getTransform();
		traktor::Transform jt1(
			m_transform * jt0.translation().xyz1(),
			jt0.rotation()
		);
		joint.setTransform(jt1);
	}
	model.setJoints(joints);

	auto animations = model.getAnimations();
	for (auto animation : animations)
	{
		for (uint32_t i = 0; i < animation->getKeyFrameCount(); ++i)
		{
			Ref< Pose > pose = DeepClone(animation->getKeyFramePose(i)).create< Pose >();

			for (uint32_t j = 0; j < model.getJointCount(); ++j)
			{
				traktor::Transform jt0 = pose->getJointTransform(j);
				traktor::Transform jt1(
					m_transform * jt0.translation().xyz1(),
					jt0.rotation()
				);
				pose->setJointTransform(j, jt1);
			}

			animation->setKeyFramePose(i, pose);
		}
	}

	return true;
}

void Transform::serialize(ISerializer& s)
{
	s >> Member< Matrix44 >(L"transform", m_transform);
}

	}
}
