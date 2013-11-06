#include <map>
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/Animation/Animation.h"
#include "Animation/Editor/AnimationFormatLws.h"
#include "Animation/Editor/SkeletonFormatLws.h"
#include "Animation/Editor/LwsParser/LwsDocument.h"
#include "Animation/Editor/LwsParser/LwsGroup.h"
#include "Animation/Editor/LwsParser/LwsValue.h"
#include "Core/Log/Log.h"
#include "Core/Math/Transform.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

struct JointTransform
{
	std::wstring name;
	float position[4];
	float rotation[4];

	JointTransform()
	{
		position[0] = position[1] = position[2] = position[3] = 0.0f;
		rotation[0] = rotation[1] = rotation[2] = rotation[3] = 0.0f;
	}
};

int32_t findJointIndex(const Skeleton* skeleton, const std::wstring& jointName)
{
	uint32_t jointCount = skeleton->getJointCount();
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		const Joint* joint = skeleton->getJoint(i);
		T_ASSERT (joint);

		if (joint->getName() == jointName)
			return int32_t(i);
	}
	return -1;
}

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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationFormatLws", AnimationFormatLws, IAnimationFormat)

Ref< Animation > AnimationFormatLws::import(IStream* stream, const Vector4& offset, bool invertX, bool invertZ) const
{
	Ref< LwsDocument > document = LwsDocument::parse(stream);
	if (!document)
		return 0;

	Ref< Skeleton > skeleton = SkeletonFormatLws().create(document);
	if (!skeleton)
		return 0;

	LwsGroup* rootGroup = document->getRootGroup();
	T_ASSERT (rootGroup);

	std::map< int32_t, std::map< int32_t, JointTransform > > keyFrames;
	std::map< std::wstring, int32_t > nameCount;
	int32_t framesPerSecond = 30;
	int32_t numChannels = 0;
	std::wstring jointName = L"";
	int32_t jointIndex = -1;

	uint32_t count = rootGroup->getCount();
	for (uint32_t i = 0; i < count; ++i)
	{
		const LwsNode* node = rootGroup->get(i);
		T_ASSERT (node);

		const LwsValue* value = dynamic_type_cast< const LwsValue* >(node);
		if (value)
		{
			if (value->getName() == L"FramesPerSecond" && value->getCount() >= 1)
				framesPerSecond = value->getInteger(0);

			if (value->getName() == L"AddBone" && value->getCount() >= 1)
				jointIndex = -1;

			if (jointIndex == -1 && value->getName() == L"BoneWeightMapName" && value->getCount() >= 1)
			{
				jointName = value->getString(0);

				int32_t jointNameIndex = nameCount[jointName]++;
				if (jointNameIndex > 0)
					jointName += L"_" + toString(jointNameIndex);

				jointIndex = findJointIndex(skeleton, jointName);
			}

			if (jointIndex != -1 && value->getName() == L"NumChannels" && value->getCount() >= 1)
			{
				numChannels = value->getInteger(0);
			}

			if (jointIndex != -1 && numChannels > 0 && value->getName() == L"Channel" && value->getCount() >= 1)
			{
				int32_t channel = value->getInteger(0);

				const LwsGroup* group = dynamic_type_cast< const LwsGroup* >(rootGroup->get(i + 1));
				if (group && group->getName() == L"Envelope")
				{
					for (uint32_t j = 1; j < group->getCount(); ++j)
					{
						const LwsValue* key = dynamic_type_cast< const LwsValue* >(group->get(j));
						if (!key || key->getName() != L"Key" || key->getCount() < 2)
							continue;

						float kt = key->getFloat(1);
						if (kt < 0.0f)
							continue;

						int32_t frame = int32_t(kt * framesPerSecond);
						JointTransform& jointTransform = keyFrames[frame][jointIndex];
						jointTransform.name = jointName;

						float kv = key->getFloat(0);
						switch (channel)
						{
						// X
						case 0:
							jointTransform.position[0] = kv;
							break;

						// Y
						case 1:
							jointTransform.position[1] = kv;
							break;

						// Z
						case 2:
							jointTransform.position[2] = kv;
							break;

						// Head
						case 3:
							jointTransform.rotation[0] = kv;
							break;

						// Pitch
						case 4:
							jointTransform.rotation[1] = kv;
							break;

						// Bank
						case 5:
							jointTransform.rotation[2] = kv;
							break;
						}
					}
				}

				if (--numChannels <= 0)
					jointIndex = -1;
			}
		}
	}

	Ref< Animation > anim = new Animation();
	for (std::map< int32_t, std::map< int32_t, JointTransform > >::const_iterator i = keyFrames.begin(); i != keyFrames.end(); ++i)
	{
		float time = float(i->first) / framesPerSecond;
		const std::map< int32_t, JointTransform >& jointTransforms = i->second;

		Animation::KeyPose keyPose;
		keyPose.at = time;

		for (std::map< int32_t, JointTransform >::const_iterator j = jointTransforms.begin(); j != jointTransforms.end(); ++j)
		{
			int32_t jointIndex = j->first;
			const JointTransform& jointTransform = j->second;

			Quaternion Qref = calculateReferenceOrientation(skeleton, jointIndex);

			Vector4 rotations[] =
			{
				Vector4(0.0f, jointTransform.rotation[0], 0.0f),
				Vector4(jointTransform.rotation[1], 0.0f, 0.0f),
				Vector4(0.0f, 0.0f, jointTransform.rotation[2])
			};
			Rotator R(
				Qref.inverse() * rotations[0],
				Qref.inverse() * rotations[1],
				Qref.inverse() * rotations[2]
			);

			const Joint* joint = skeleton->getJoint(jointIndex);
			T_ASSERT (joint);

			Vector4 Prest = joint->getTransform().translation();

			Vector4 P = Vector4::loadUnaligned(jointTransform.position).xyz0() - Prest;

			keyPose.pose.setJointOffset(jointIndex, P);
			keyPose.pose.setJointOrientation(jointIndex, R);
		}

		anim->addKeyPose(keyPose);
	}

	return anim;
}

	}
}
