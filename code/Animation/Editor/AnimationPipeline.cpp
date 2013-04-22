#include "Animation/Animation/Animation.h"
#include "Animation/Editor/AnimationAsset.h"
#include "Animation/Editor/AnimationFormatBvh.h"
#include "Animation/Editor/AnimationPipeline.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineSettings.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationPipeline", 2, AnimationPipeline, editor::IPipeline)

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
	return true;
}

bool AnimationPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
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
		animationAsset->getInvertZ()
	);

	file->close();

	if (!anim)
	{
		log::error << L"Unable to build animation; import failed \"" << fileName.getPathName() << L"\"" << Endl;
		return false;
	}

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
