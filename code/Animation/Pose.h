/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/BitSet.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

/*! Skeleton pose.
 * \ingroup Animation
 */
class T_DLLCLASS Pose : public ISerializable
{
	T_RTTI_CLASS;

public:
	void reset();

	void reserve(uint32_t jointCapacity);

	void setJointTransform(uint32_t jointIndex, const Transform& jointTransform);

	Transform getJointTransform(uint32_t jointIndex) const;

	uint32_t getMaxIndex() const;

	void getIndexMask(BitSet& outIndices) const;

	virtual void serialize(ISerializer& s) override final;

private:
	struct Joint
	{
		uint32_t index;
		Transform transform;

		Joint(uint32_t index_ = 0)
		:	index(index_)
		,	transform(Transform::identity())
		{
		}

		void serialize(ISerializer& s);
	};

	AlignedVector< Joint > m_joints;

	const Joint* getJoint(uint32_t jointIndex) const;

	Joint& getEditJoint(uint32_t jointIndex);
};

}
