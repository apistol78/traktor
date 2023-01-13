/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/Animation.h"
#include "Animation/Editor/AnimationAsset.h"
#include "Animation/Editor/AnimationPipeline.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Model/Joint.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/Pose.h"
#include "Model/Operations/Transform.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationPipeline", 13, AnimationPipeline, editor::IPipeline)

bool AnimationPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	m_modelCachePath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.ModelCache.Path");
	return true;
}

void AnimationPipeline::destroy()
{
}

TypeInfoSet AnimationPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< AnimationAsset >();
}

uint32_t AnimationPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool AnimationPipeline::shouldCache() const
{
	return true;
}

bool AnimationPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< const AnimationAsset > animationAsset = checked_type_cast< const AnimationAsset* >(sourceAsset);

	// Animation source data.
	pipelineDepends->addDependency(m_assetPath, animationAsset->getFileName().getPathName());

	// Remapping skeleton if skeleton in animation data differ from mesh skeleton.
	if (animationAsset->getSkeleton().isNotNull())
		pipelineDepends->addDependency(animationAsset->getSkeleton(), editor::PdfUse);

	return true;
}

bool AnimationPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	Ref< const AnimationAsset > animationAsset = checked_type_cast< const AnimationAsset* >(sourceAsset);

	// Read source model.
	const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + animationAsset->getFileName());
	Ref< model::Model > modelAnimation = model::ModelCache(m_modelCachePath).get(filePath, L"");
	if (!modelAnimation)
	{
		log::error << L"Unable to build animation; no such file \"" << animationAsset->getFileName().getPathName() << L"\"." << Endl;
		return false;
	}

	// Read skeleton model.
	Ref< model::Model > modelSkeleton = modelAnimation;
	if (animationAsset->getSkeleton().isNotNull())
	{
		Ref< const SkeletonAsset > skeletonAsset = pipelineBuilder->getObjectReadOnly< SkeletonAsset >(animationAsset->getSkeleton());
		if (!skeletonAsset)
		{
			log::error << L"Unable to build animation; no such skeleton asset." << Endl;
			return false;
		}

		const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + skeletonAsset->getFileName());
		modelSkeleton = model::ModelCache(m_modelCachePath).get(filePath, L"");
		if (!modelSkeleton)
		{
			log::error << L"Unable to build animation; no such file \"" << skeletonAsset->getFileName().getPathName() << L"\"." << Endl;
			return false;
		}
	}

	// Scale and/or translate animation data.
	model::Transform(
		translate(animationAsset->getTranslate()) *
		scale(animationAsset->getScale(), animationAsset->getScale(), animationAsset->getScale())
	).apply(*modelAnimation);

	// Set joint orientations in animation rest/poses to match skeleton.
	for (uint32_t i = 0; i < modelSkeleton->getJointCount(); ++i)
	{
		const model::Joint& jointSkeleton = modelSkeleton->getJoint(i);

		const uint32_t jointIdx = modelAnimation->findJointIndex(jointSkeleton.getName());
		if (jointIdx == model::c_InvalidIndex)
		{
			log::warning << L"No such joint \"" << jointSkeleton.getName() << L"\" in animation skeleton." << Endl;
			continue;
		}

		const Quaternion rotation = jointSkeleton.getTransform().rotation();
		modelAnimation->setJointRotation(jointIdx, rotation);
	}

	// Find animation take.
	const std::wstring take = animationAsset->getTake();

	Ref< const model::Animation > ma = modelAnimation->findAnimation(take);
	if (!ma)
	{
		log::error << L"Unable to build animation; no such animation \"" << take << L"\" in file \"" << animationAsset->getFileName().getPathName() << L"\"." << Endl;
		log::error << L"Available animations are:" << Endl;
		for (const auto anim : modelAnimation->getAnimations())
			log::error << L"\t\"" << anim->getName() << L"\"" << Endl;
		return false;
	}

	Ref< Animation > anim = new Animation();

	// Generate key poses; retarget animations onto skeleton mesh.
	const AlignedVector< model::Joint >& skeletonMeshJoints = modelSkeleton->getJoints();
	const AlignedVector< model::Joint >& skeletonAnimJoints = modelAnimation->getJoints();
	for (uint32_t i = 0; i < ma->getKeyFrameCount(); ++i)
	{
		const float time = ma->getKeyFrameTime(i);
		const model::Pose* mp = ma->getKeyFramePose(i);

		Animation::KeyPose kp;
		kp.at = time;

		for (uint32_t j = 0; j < skeletonMeshJoints.size(); ++j)
		{
			const std::wstring& name = skeletonMeshJoints[j].getName();

			const uint32_t k = modelAnimation->findJointIndex(name);
			if (k == model::c_InvalidIndex)
			{
				log::warning << L"No \"" << name << L"\" joint in animation skeleton." << Endl;
				continue;
			}

			const Transform TposeAnim = mp->getJointTransform(k);

			kp.pose.setJointTransform(
				j,
				TposeAnim
			);
		}

		anim->addKeyPose(kp);
	}

	// Remove locomotion from animation.
	//
	// \notes
	//   Currently assumes joint 0 is root joint and are
	//   used to measure locomotion.
	//
	if (animationAsset->getRemoveLocomotion())
	{
		const uint32_t keyPoseCount = anim->getKeyPoseCount();
		if (keyPoseCount >= 2)
		{
			// Create skeleton from model.
			Ref< Skeleton > skeleton = new Skeleton();
			for (const auto& modelJoint : modelSkeleton->getJoints())
			{
				Ref< Joint > joint = new Joint();

				if (modelJoint.getParent() != model::c_InvalidIndex)
					joint->setParent(modelJoint.getParent());

				joint->setName(modelJoint.getName());
				joint->setTransform(modelJoint.getTransform());

				skeleton->addJoint(joint);
			}

			AlignedVector< Transform > poseTransforms;
			calculatePoseTransforms(skeleton, &anim->getKeyPose(0).pose, poseTransforms);

			float locomotionDistance = 0.0f;

			const Transform origin = poseTransforms[0];
			Vector4 previousTarget = origin.translation();
			for (uint32_t i = 1; i < keyPoseCount; ++i)
			{
				auto& keyPose = anim->getKeyPose(i);

				AlignedVector< Transform > targetPoseTransforms;
				calculatePoseTransforms(skeleton, &keyPose.pose, targetPoseTransforms);

				const Transform target = targetPoseTransforms[0];
				const Vector4 locomotion = (target.translation() - origin.translation()) * Vector4(1.0f, 0.0f, 1.0f, 0.0f);

				for (uint32_t i = 0; i < skeletonMeshJoints.size(); ++i)
					targetPoseTransforms[i] = Transform(-locomotion) * targetPoseTransforms[i];

				// Convert back from absolute to relative pose transforms.
				for (uint32_t i = 0; i < skeletonMeshJoints.size(); ++i)
				{
					const int32_t parentIdx = skeleton->getJoint(i)->getParent();
					const Transform parentTransform = (parentIdx >= 0) ? targetPoseTransforms[parentIdx] : Transform::identity();
					keyPose.pose.setJointTransform(i, parentTransform.inverse() * targetPoseTransforms[i]);
				}

				// Accumulate locomotion distance.
				locomotionDistance += ((target.translation() - previousTarget) * Vector4(1.0f, 0.0f, 1.0f, 0.0f)).length();
				previousTarget = target.translation();
			}

			const float duration = anim->getLastKeyPose().at - anim->getKeyPose(0).at;
			anim->setTimePerDistance(duration / locomotionDistance);
		}
	}

	/*
	// Discard redundant key poses.
	uint32_t uncompressedCount = anim->getKeyPoseCount();
	if (uncompressedCount > 2)
	{
		bool anyRemoved = true;

		while (anyRemoved)
		{
			anyRemoved = false;

			int32_t indexHint = -1;

			uint32_t count = anim->getKeyPoseCount();
			for (uint32_t i = 0; i < count - 2; ++i)
			{
				Animation::KeyPose k0 = anim->getKeyPose(i);
				Animation::KeyPose k1 = anim->getKeyPose(i + 1);
				Animation::KeyPose k2 = anim->getKeyPose(i + 2);

				uint32_t jointCount = max(max(k0.pose.getMaxIndex(), k1.pose.getMaxIndex()), k2.pose.getMaxIndex()) + 1;

				// Evaluate reference path.
				AlignedVector< Pose > ref;

				indexHint = -1;
				for (float T = k0.at; T <= k2.at; T += 1.0f / 60.0f)
				{
					Pose pose;
					anim->getPose(T, false, indexHint, pose);
					ref.push_back(pose);
				}

				// Remove middle key pose and re-evaluate.
				anim->removeKeyPose(i + 1);

				AlignedVector< Pose >::const_iterator ir = ref.begin();

				Scalar totalOffsetError(0.0f);
				Scalar totalOrientationError(0.0f);

				indexHint = -1;
				for (float T = k0.at; T <= k2.at; T += 1.0f / 60.0f)
				{
					if (ir == ref.end())
					{
						totalOffsetError = Scalar(std::numeric_limits< float >::max());
						totalOrientationError = Scalar(std::numeric_limits< float >::max());
						break;
					}

					Pose pose;
					anim->getPose(T, false, indexHint, pose);

					for (uint32_t j = 0; j < jointCount; ++j)
					{
						Transform jointTransformRef = ir->getJointTransform(j);
						Transform jointTransformCheck = pose.getJointTransform(j);

						Scalar offsetError = (jointTransformRef.translation() - jointTransformCheck.translation()).length();
						Scalar orientationError = (jointTransformRef.rotation().inverse() * jointTransformCheck.rotation()).toAxisAngle().length();

						totalOffsetError += offsetError;
						totalOrientationError += orientationError;
					}

					++ir;
				}

				if (totalOffsetError > FUZZY_EPSILON || totalOrientationError > FUZZY_EPSILON)
					anim->addKeyPose(k1);
				else
				{
					anyRemoved = true;
					break;
				}
			}
		}
	}

	if (uncompressedCount != anim->getKeyPoseCount())
		log::info << L"Removed " << (uncompressedCount - anim->getKeyPoseCount()) << L" redundant key poses in animation; was " << uncompressedCount << L", now " << anim->getKeyPoseCount() << Endl;
	*/

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!instance)
	{
		log::error << L"Unable to build animation; unable to create output instance \"" << outputPath << L"\"." << Endl;
		return false;
	}

	instance->setObject(anim);

	if (!instance->commit())
	{
		log::error << L"Unable to build animation; unable to commit output instance \"" << outputPath << L"\"." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > AnimationPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

	}
}
