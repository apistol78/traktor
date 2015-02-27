#include <limits>
#include "Animation/Animation/Animation.h"
#include "Animation/Editor/AnimationAsset.h"
#include "Animation/Editor/AnimationFormatBvh.h"
#include "Animation/Editor/AnimationFormatLws.h"
#include "Animation/Editor/AnimationPipeline.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationPipeline", 9, AnimationPipeline, editor::IPipeline)

bool AnimationPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void AnimationPipeline::destroy()
{
}

TypeInfoSet AnimationPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< AnimationAsset >());
	return typeSet;
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
	pipelineDepends->addDependency(m_assetPath, animationAsset->getFileName().getPathName());
	return true;
}

bool AnimationPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	Ref< const AnimationAsset > animationAsset = checked_type_cast< const AnimationAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, animationAsset->getFileName());

	Ref< IAnimationFormat > format;

	if (compareIgnoreCase< std::wstring >(fileName.getExtension(), L"bvh") == 0)
		format = new AnimationFormatBvh();
	else if (compareIgnoreCase< std::wstring >(fileName.getExtension(), L"lws") == 0)
		format = new AnimationFormatLws();

	if (!format)
	{
		log::error << L"Unable to build animation; unsupported format \"" << fileName.getExtension() << L"\"" << Endl;
		return false;
	}

	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
	{
		log::error << L"Unable to build animation; no such file \"" << fileName.getPathName() << L"\"" << Endl;
		return false;
	}

	Ref< Animation > anim = format->import(
		file, 
		animationAsset->getOffset(),
		animationAsset->getInvertX(),
		animationAsset->getInvertZ(),
		animationAsset->shouldAutoCenterKeyPoses()
	);

	file->close();

	if (!anim)
	{
		log::error << L"Unable to build animation; import failed \"" << fileName.getPathName() << L"\"" << Endl;
		return false;
	}

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
						Vector4 jointOffsetRef = ir->getJointOffset(j);
						Vector4 jointOffsetCheck = pose.getJointOffset(j);

						Rotator jointOrientationRef = ir->getJointOrientation(j);
						Rotator jointOrientationCheck = pose.getJointOrientation(j);

						Scalar offsetError = (jointOffsetRef - jointOffsetCheck).length();
						Scalar orientationError = (jointOrientationRef.toQuaternion().inverse() * jointOrientationCheck.toQuaternion()).toAxisAngle().length();

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

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!instance)
	{
		log::error << L"Unable to build animation; unable to create output instance \"" << outputPath << L"\"" << Endl;
		return false;
	}

	instance->setObject(anim);

	if (!instance->commit())
	{
		log::error << L"Unable to build animation; unable to commit output instance \"" << outputPath << L"\"" << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > AnimationPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
