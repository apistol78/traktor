/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Model/Model.h"
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
	return jointId < m_jointTransforms.size() ? m_jointTransforms[jointId] : Transform::identity();
}

Transform Pose::getJointGlobalTransform(const Model* model, uint32_t jointId) const
{
	const auto& joints = model->getJoints();

	Transform Tglobal = Transform::identity();
	while (jointId != c_InvalidIndex)
	{
		Tglobal = getJointTransform(jointId) * Tglobal;	// ABC order (A root)
		jointId = joints[jointId].getParent();
	}

	return Tglobal;
}

void Pose::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Transform, MemberComposite< Transform > >(L"jointTransforms", m_jointTransforms);
}

	}
}
