/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
	if (!animations.empty())
	{
		Matrix44 transformZeroOffset(
			m_transform.axisX(),
			m_transform.axisY(),
			m_transform.axisZ(),
			Vector4::origo()
		);

		for (auto animation : animations)
		{
			for (uint32_t i = 0; i < animation->getKeyFrameCount(); ++i)
			{
				Ref< Pose > pose = new Pose(*animation->getKeyFramePose(i));
				for (uint32_t j = 0; j < model.getJointCount(); ++j)
				{
					traktor::Transform jt0 = pose->getJointTransform(j);
					if (model.getJoint(j).getParent() != c_InvalidIndex)
					{
						traktor::Transform jt1(
							transformZeroOffset * jt0.translation().xyz1(),
							jt0.rotation()
						);
						pose->setJointTransform(j, jt1);
					}
					else
					{
						traktor::Transform jt1(
							m_transform * jt0.translation().xyz1(),
							jt0.rotation()
						);
						pose->setJointTransform(j, jt1);
					}
				}
				animation->setKeyFramePose(i, pose);
			}
		}
	}

	return true;
}

	}
}
