#include "Animation/Skeleton.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Animation/Editor/SkeletonFormatBvh.h"
#include "Animation/Editor/SkeletonFormatLws.h"
#include "Animation/Editor/SkeletonPipeline.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.SkeletonPipeline", 3, SkeletonPipeline, editor::IPipeline)

bool SkeletonPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void SkeletonPipeline::destroy()
{
}

TypeInfoSet SkeletonPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SkeletonAsset >());
	return typeSet;
}

bool SkeletonPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const SkeletonAsset > skeletonAsset = checked_type_cast< const SkeletonAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, skeletonAsset->getFileName());

	pipelineDepends->addDependency(fileName);

	return true;
}

bool SkeletonPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< const SkeletonAsset > skeletonAsset = checked_type_cast< const SkeletonAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, skeletonAsset->getFileName());

	Ref< ISkeletonFormat > format;

	if (compareIgnoreCase< std::wstring >(fileName.getExtension(), L"bvh") == 0)
		format = new SkeletonFormatBvh();
	else if (compareIgnoreCase< std::wstring >(fileName.getExtension(), L"lws") == 0)
		format = new SkeletonFormatLws();

	if (!format)
	{
		log::error << L"Unable to build skeleton; unsupported format" << Endl;
		return false;
	}

	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
	{
		log::error << L"Unable to build skeleton; no such file" << Endl;
		return false;
	}

	Ref< Skeleton > skeleton = format->import(
		file,
		skeletonAsset->getOffset(),
		skeletonAsset->getRadius(),
		skeletonAsset->getInvertX(),
		skeletonAsset->getInvertZ()
	);

	file->close();

	if (!skeleton)
	{
		log::error << L"Unable to build skeleton; import failed" << Endl;
		return false;
	}

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!instance)
	{
		log::error << L"Unable to build skeleton; unable to create output instance" << Endl;
		return false;
	}

	instance->setObject(skeleton);

	if (!instance->commit())
	{
		log::error << L"Unable to build skeleton; unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > SkeletonPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
