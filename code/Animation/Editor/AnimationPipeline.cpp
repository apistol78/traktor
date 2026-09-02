/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
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
#include "Core/Settings/PropertyBoolean.h"
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

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationPipeline", 22, AnimationPipeline, editor::IPipeline)

bool AnimationPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
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

	// Retarget skeleton.
	if (animationAsset->getTargetSkeleton().isNotNull())
		pipelineDepends->addDependency(animationAsset->getTargetSkeleton(), editor::PdfUse);

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
	Ref< model::Model > modelAnimation = model::ModelCache::getInstance().getMutable(m_modelCachePath, filePath, L"");
	if (!modelAnimation)
	{
		log::error << L"Unable to build animation; no such file \"" << animationAsset->getFileName().getPathName() << L"\"." << Endl;
		return false;
	}

	// Read skeleton model.
	Ref< model::Model > modelSkeleton = modelAnimation;
	if (animationAsset->getTargetSkeleton().isNotNull())
	{
		Ref< const SkeletonAsset > skeletonAsset = pipelineBuilder->getObjectReadOnly< SkeletonAsset >(animationAsset->getTargetSkeleton());
		if (!skeletonAsset)
		{
			log::error << L"Unable to build animation; no such skeleton asset." << Endl;
			return false;
		}

		const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + skeletonAsset->getFileName());
		modelSkeleton = model::ModelCache::getInstance().getMutable(m_modelCachePath, filePath, L"");
		if (!modelSkeleton)
		{
			log::error << L"Unable to build animation; no such file \"" << skeletonAsset->getFileName().getPathName() << L"\"." << Endl;
			return false;
		}
	}

	// Scale and/or translate animation data.
	modelAnimation->apply(model::Transform(
		translate(animationAsset->getTranslate()) *
		scale(animationAsset->getScale())
	));

	// Match joint rest orientations between the animation's rig and the skeleton.
	// Orientations are matched in global space; the animation's rig might not have
	// the same root as the skeleton's, in which case local orientations of the
	// animation's root joint are expressed relative to another parent.
	//
	// Key poses replace joint transforms entirely (model::Pose is absolute, not
	// bind-relative), so re-orienting the animation rig's bind pose never reaches the
	// transferred poses. Instead compute, per joint, the rest-orientation delta between
	// the rigs and apply it while transferring poses below:
	//
	//   skeletonGlobal(t) = animGlobal(t) * animRestGlobal^-1 * skeletonRestGlobal
	//
	// At rest this lands exactly on the skeleton's bind orientation while following the
	// animation's global-space rotation over time; for rigs sharing bind conventions the
	// correction is identity.
	//
	// Only possible when the animation's rig has an actual rest pose to measure against.
	// Rigs imported from files without geometry have no bind pose; their rest is merely
	// the scene's static pose, and matching against it would bake a constant orientation
	// error into every joint, i.e. correct joint positions but skewed skinning. Such rigs
	// are instead assumed to already share the skeleton's rest pose.
	const bool matchOrientations = modelAnimation->getProperty< bool >(L"JointsFromBindPose", true);
	if (!matchOrientations)
		log::info << L"Animation's rig has no bind pose; assuming it shares rest pose with skeleton." << Endl;

	AlignedVector< Quaternion > restCorrections(modelSkeleton->getJointCount(), Quaternion::identity());
	for (uint32_t i = 0; i < modelSkeleton->getJointCount(); ++i)
	{
		const model::Joint& jointSkeleton = modelSkeleton->getJoint(i);

		const uint32_t jointIdx = modelAnimation->findJointIndex(jointSkeleton.getName());
		if (jointIdx == model::c_InvalidIndex)
		{
			log::warning << L"No such joint \"" << jointSkeleton.getName() << L"\" in animation skeleton." << Endl;
			continue;
		}

		if (!matchOrientations)
			continue;

		const Quaternion QanimRest = modelAnimation->getJointGlobalTransform(jointIdx).rotation();
		const Quaternion QskeletonRest = modelSkeleton->getJointGlobalTransform(i).rotation();
		restCorrections[i] = QanimRest.inverse() * QskeletonRest;
	}

	// Find animation take.
	log::info << L"Available animations are:" << Endl;
	for (const auto anim : modelAnimation->getAnimations())
		log::info << L"\t\"" << anim->getName() << L"\", " << anim->getKeyFrameCount() << L" frame(s)" << Endl;

	const std::wstring take = animationAsset->getTake();

	Ref< const model::Animation > ma;
	if (!take.empty())
		ma = modelAnimation->findAnimation(take);
	else if (modelAnimation->getAnimationCount() > 0)
		ma = modelAnimation->getAnimation(0);

	if (!ma)
	{
		log::error << L"Unable to build animation; no such animation \"" << take << L"\" in file \"" << animationAsset->getFileName().getPathName() << L"\"." << Endl;
		return false;
	}

	Ref< Animation > anim = new Animation();

	// Optionally cut the take short; key frames later than the maximum duration are
	// discarded. Measured from the take's first key frame, so takes which do not start
	// at zero are cut after the same amount of animation. A maximum duration of zero
	// means no limit; the first key frame is always kept.
	const float maxDuration = animationAsset->getMaxDuration();
	const float takeAt = (ma->getKeyFrameCount() > 0) ? ma->getKeyFrameTime(0) : 0.0f;

	// Determine which animation joints actually animate their local translation.
	// Joints that do (typically the hips, carrying locomotion and bob) keep the
	// animation's translations; all others use the target skeleton's bind translations
	// so the skeleton's own bone lengths and proportions are preserved when the rigs
	// differ. For rigs sharing proportions this is a no-op, as a static local
	// translation equals the bind translation.
	const AlignedVector< model::Joint >& skeletonMeshJoints = modelSkeleton->getJoints();
	AlignedVector< uint8_t > translationAnimated(skeletonMeshJoints.size(), 0);
	if (ma->getKeyFrameCount() > 0)
	{
		for (uint32_t j = 0; j < skeletonMeshJoints.size(); ++j)
		{
			const uint32_t k = modelAnimation->findJointIndex(skeletonMeshJoints[j].getName());
			if (k == model::c_InvalidIndex)
				continue;

			const Vector4 T0 = ma->getKeyFramePose(0)->getJointTransform(k).translation();
			for (uint32_t i = 1; i < ma->getKeyFrameCount(); ++i)
			{
				if ((ma->getKeyFramePose(i)->getJointTransform(k).translation() - T0).length() > 0.0001f)
				{
					translationAnimated[j] = 1;
					break;
				}
			}
		}
	}

	// Generate key poses; retarget animations onto skeleton mesh.
	// Poses are transferred in global space since the animation's rig might not share
	// hierarchy with the skeleton, most commonly by lacking the skeleton's root joint;
	// local transformations of such a rig's root are relative another parent and thus
	// cannot be copied verbatim. For rigs sharing hierarchy this is a no-op.
	AlignedVector< Transform > poseGlobalTransforms(skeletonMeshJoints.size(), Transform::identity());
	for (uint32_t i = 0; i < ma->getKeyFrameCount(); ++i)
	{
		const float time = ma->getKeyFrameTime(i);
		if (maxDuration > 0.0f && time - takeAt > maxDuration)
			continue;

		const model::Pose* mp = ma->getKeyFramePose(i);

		Animation::KeyPose kp;
		kp.at = time;

		for (uint32_t j = 0; j < skeletonMeshJoints.size(); ++j)
		{
			const std::wstring& name = skeletonMeshJoints[j].getName();

			// Joints are always ordered parent before child thus parent's global
			// transformation of this pose is already known.
			const uint32_t parent = skeletonMeshJoints[j].getParent();
			const Transform Tparent = (parent != model::c_InvalidIndex) ? poseGlobalTransforms[parent] : Transform::identity();

			Transform TposeJoint;

			const uint32_t k = modelAnimation->findJointIndex(name);
			if (k != model::c_InvalidIndex)
			{
				const Transform Tanim = mp->getJointGlobalTransform(modelAnimation, k);
				TposeJoint = Tparent.inverse() * Transform(Tanim.translation(), Tanim.rotation() * restCorrections[j]);

				// Preserve the skeleton's own bone lengths; only translation-animated
				// joints (locomotion, bob) keep the animation's translation.
				if (!translationAnimated[j])
					TposeJoint = Transform(skeletonMeshJoints[j].getTransform().translation(), TposeJoint.rotation());
			}
			else
			{
				// Joint is missing from the source animation (already warned about above);
				// keep it at the target skeleton's bind pose rather than collapsing to identity.
				TposeJoint = skeletonMeshJoints[j].getTransform();
			}

			poseGlobalTransforms[j] = Tparent * TposeJoint;
			kp.pose.setJointTransform(j, TposeJoint);
		}

		anim->addKeyPose(kp);
	}

	if (maxDuration > 0.0f && anim->getKeyPoseCount() < ma->getKeyFrameCount())
	{
		log::info << L"Animation cut at " << maxDuration << L" second(s); kept " << anim->getKeyPoseCount() <<
			L" of " << ma->getKeyFrameCount() << L" key frame(s)." << Endl;
	}

	// Remove locomotion from animation.
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

			// Get joint handle for reference joint; if none named
			// then assume first joint for reference.
			uint32_t jointIndex = 0;
			if (!animationAsset->getRemoveLocomotionJoint().empty())
			{
				if (!skeleton->findJoint(render::getParameterHandle(animationAsset->getRemoveLocomotionJoint()), jointIndex))
				{
					log::error << L"AnimationPipeline failed; unable to remove locomotion, no such joint \"" << animationAsset->getRemoveLocomotionJoint() << L"\"." << Endl;
					return false;
				}
			}

			AlignedVector< Transform > poseTransforms;
			calculatePoseTransforms(skeleton, &anim->getKeyPose(0).pose, poseTransforms);

			Vector4 totalLocomotion = Vector4::zero();
			float locomotionDistance = 0.0f;

			const Transform origin = poseTransforms[0];
			Vector4 previousTarget = origin.translation();
			for (uint32_t i = 1; i < keyPoseCount; ++i)
			{
				const Vector4 c_locomotionAxies(0.0f, 0.0f, 1.0f);
				auto& keyPose = anim->getKeyPose(i);

				AlignedVector< Transform > targetPoseTransforms;
				calculatePoseTransforms(skeleton, &keyPose.pose, targetPoseTransforms);

				const Transform target = targetPoseTransforms[jointIndex];
				const Vector4 locomotion = (target.translation() - origin.translation()) * c_locomotionAxies;

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
				const Vector4 deltaLocomotion = (target.translation() - previousTarget) * c_locomotionAxies;
				locomotionDistance += deltaLocomotion.length();
				totalLocomotion += deltaLocomotion;
				previousTarget = target.translation();
			}

			const float duration = anim->getLastKeyPose().at - anim->getKeyPose(0).at;
			anim->setTimePerDistance(std::abs(locomotionDistance) > FUZZY_EPSILON ? duration / locomotionDistance : 0.0f);
			anim->setTotalLocomotion(totalLocomotion);
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
