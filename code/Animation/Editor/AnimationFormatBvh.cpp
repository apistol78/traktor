#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Misc/String.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/Animation.h"
#include "Animation/Editor/AnimationFormatBvh.h"
#include "Animation/Editor/SkeletonFormatBvh.h"
#include "Animation/Editor/BvhParser/BvhDocument.h"
#include "Animation/Editor/BvhParser/BvhJoint.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

Quaternion calculateReferenceOrientation(const Skeleton* skeleton, int32_t jointIndex)
{
	if (jointIndex >= 0)
	{
		const Joint* joint = skeleton->getJoint(jointIndex);
		T_ASSERT (joint);
		Quaternion QworldParent = calculateReferenceOrientation(skeleton, joint->getParent());
		Quaternion Qlocal = joint->getTransform().rotation();
		return QworldParent * Qlocal;
	}
	else
		return Quaternion::identity();
}

void convertKeyPose(
	const Skeleton* skeleton,
	const BvhJoint* joint,
	const BvhDocument::cv_t& cv,
	Animation::KeyPose& kp,
	int32_t& jointIndex,
	const Vector4& modifier
)
{
	const RefArray< BvhJoint >& children = joint->getChildren();
	for (RefArray< BvhJoint >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		const BvhJoint* childJoint = *i;

		const std::vector< std::wstring >& ch = joint->getChannels();
		int32_t offset = joint->getChannelOffset();

		Vector4 P = Vector4::zero();
		Quaternion QlocalOrientation = Quaternion::identity();

		StaticVector< Vector4, 3 > rotations;

		for (std::vector< std::wstring >::const_iterator k = ch.begin(); k != ch.end(); ++k)
		{
			float c = cv[offset++];

			if (*k == L"Xposition")
				P += Vector4(c, 0.0f, 0.0f, 0.0f) * modifier;
			else if (*k == L"Yposition")
				P += Vector4(0.0f, c, 0.0f, 0.0f) * modifier;
			else if (*k == L"Zposition")
				P += Vector4(0.0f, 0.0f, c, 0.0f) * modifier;

			else if (*k == L"Xrotation")
				rotations.push_back(Vector4(deg2rad(c), 0.0f, 0.0f, 0.0f));
			else if (*k == L"Yrotation")
				rotations.push_back(Vector4(0.0f, deg2rad(-c), 0.0f, 0.0f));
			else if (*k == L"Zrotation")
				rotations.push_back(Vector4(0.0f, 0.0f, deg2rad(-c), 0.0f));
		}

		while (rotations.size() < 3)
			rotations.push_back(Vector4::zero());

		Quaternion Qref = calculateReferenceOrientation(skeleton, jointIndex);

		Rotator R(
			Qref.inverse() * rotations[0],
			Qref.inverse() * rotations[1],
			Qref.inverse() * rotations[2]
		);

		kp.pose.setJointOffset(
			jointIndex,
			P
		);

		kp.pose.setJointOrientation(
			jointIndex,
			R
		);

		jointIndex++;

		convertKeyPose(
			skeleton,
			childJoint,
			cv,
			kp,
			jointIndex,
			modifier
		);
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationFormatBvh", AnimationFormatBvh, IAnimationFormat)

Ref< Animation > AnimationFormatBvh::import(IStream* stream, const Vector4& offset, bool invertX, bool invertZ, bool autoCenterKeyPoses) const
{
	const float c_skeletonjointRadius = 0.25f;
	const Vector4 c_jointModifier(
		invertX ? -1.0f : 1.0f, 
		1.0f, 
		invertZ ? -1.0f : 1.0f, 
		1.0f
	);

	Ref< BvhDocument > document = BvhDocument::parse(stream, c_jointModifier);
	if (!document)
		return 0;

	// Create reference skeleton from BVH hierarchy; use
	// SkeletonFormatBvh class to ensure consistency with skeleton pipeline.
	SkeletonFormatBvh skeletonFormat;
	Ref< Skeleton > skeleton = skeletonFormat.create(document, offset, c_skeletonjointRadius);
	if (!skeleton)
		return 0;

	// Create key poses from BVH motions.
	Ref< Animation > anim = new Animation();

	float at = 0.0f;

	const BvhDocument::cv_list_t& cvl = document->getChannelValues();
	for (BvhDocument::cv_list_t::const_iterator i = cvl.begin(); i != cvl.end(); ++i)
	{
		Animation::KeyPose kp;
		kp.at = at;

		int32_t jointIndex = 0;
		convertKeyPose(
			skeleton,
			document->getRootJoint(),
			*i,
			kp,
			jointIndex,
			c_jointModifier
		);

		anim->addKeyPose(kp);

		at += document->getFrameTime();
	}

	if (autoCenterKeyPoses)
	{
		uint32_t poseCount = anim->getKeyPoseCount();
		if (poseCount > 1)
		{
			uint32_t jointCount = skeleton->getJointCount();
			for (uint32_t i = 0; i < jointCount; ++i)
			{
				Vector4 offset = anim->getKeyPose(0).pose.getJointOffset(i);
				for (uint32_t j = 0; j < poseCount; ++j)
				{
					Vector4 poseOffset = anim->getKeyPose(j).pose.getJointOffset(i);
					anim->getKeyPose(j).pose.setJointOffset(i, poseOffset - offset);
				}
			}
		}
	}

	return anim;
}

	}
}
