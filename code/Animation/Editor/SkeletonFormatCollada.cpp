/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <map>
#include "Animation/Skeleton.h"
#include "Animation/Editor/SkeletonFormatCollada.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{
/*
Vector4 getJointPosition(const BvhJoint* joint)
{
	Vector4 position(0.0f, 0.0f, 0.0f, 1.0f);

	if (joint->getParent())
		position = getJointPosition(joint->getParent());

	position += joint->getOffset().xyz0();
	return position;
}

void createBones(
	Skeleton* skeleton,
	const BvhJoint* joint,
	int32_t parent,
	const Quaternion& QworldParent
)
{
	std::map< std::wstring, int32_t > boneNameCount;

	const RefArray< BvhJoint >& children = joint->getChildren();
	for (RefArray< BvhJoint >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		const BvhJoint* childJoint = *i;

		std::wstring boneName = childJoint->getName();
		if (childJoint->getChildren().empty())
			boneName = joint->getName() + L"_END";

		Vector4 boneP0 = getJointPosition(joint);
		Vector4 boneP1 = getJointPosition(childJoint);

		Vector4 boneDirection = boneP1 - boneP0;
		Scalar boneLength = boneDirection.length();

		if (boneLength > Scalar(FUZZY_EPSILON))
			boneDirection /= boneLength;
		else
			boneDirection = Vector4(0.0f, 0.0f, 1.0f);

		Ref< Bone > bone = new Bone();

		bone->setName(boneName);
		bone->setParent(parent);

		if (joint->getParent())
			bone->setPosition(Vector4::origo());
		else
			bone->setPosition(boneP0);

		Vector4 VboneDirectionInParent = QworldParent.inverse() * boneDirection;

		float head, pitch;
		if (VboneDirectionInParent.z() >= 0.0f)
		{
			head = atan2f(VboneDirectionInParent.x(), VboneDirectionInParent.z());
			pitch = acosf(VboneDirectionInParent.y()) - HALF_PI;
		}
		else
		{
			head = atan2f(-VboneDirectionInParent.x(), -VboneDirectionInParent.z());
			pitch = -acosf(VboneDirectionInParent.y()) - HALF_PI;
		}

		Quaternion Qlocal = Quaternion(Vector4(0.0f, head, 0.0f)) * Quaternion(Vector4(pitch, 0.0f, 0.0f));
		Quaternion Qworld = QworldParent * Qlocal;

		bone->setOrientation(Qlocal);

		bone->setLength(boneLength);
		bone->setRadius(Scalar(0.25f));
		bone->setEnableLimits(false);

		int32_t index = skeleton->addBone(bone);

		createBones(
			skeleton,
			childJoint,
			index,
			Qworld
		);
	}
}
*/
		}
/*
T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SkeletonFormatCollada", SkeletonFormatCollada, ISkeletonFormat)

Ref< Skeleton > SkeletonFormatCollada::create(const BvhDocument* document) const
{
	Ref< Skeleton > skeleton = new Skeleton();
	createBones(
		skeleton,
		document->getRootJoint(),
		-1,
		Quaternion::identity()
	);
	return skeleton;
}
*/
Ref< Skeleton > SkeletonFormatCollada::import(IStream* stream, const Vector4& offset, float scale, float radius, bool invertX, bool invertZ) const
{
/*
	Ref< BvhDocument > document = BvhDocument::parse(stream);
	if (!document)
		return 0;

	Ref< Skeleton > skeleton = create(document);

	if (skeleton)
		log::info << L"Created " << skeleton->getBoneCount() << L" bone(s) in skeleton" << Endl;

	return skeleton;
*/
	Ref< Skeleton > skeleton;
	return skeleton;
}

	}
}
