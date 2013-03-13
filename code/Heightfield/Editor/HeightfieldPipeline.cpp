#include <limits>
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldResource.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldFormat.h"
#include "Heightfield/Editor/HeightfieldPipeline.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldPipeline", 1, HeightfieldPipeline, editor::IPipeline)

bool HeightfieldPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void HeightfieldPipeline::destroy()
{
}

TypeInfoSet HeightfieldPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< HeightfieldAsset >());
	return typeSet;
}

bool HeightfieldPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< const HeightfieldAsset > heightfieldAsset = checked_type_cast< const HeightfieldAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), heightfieldAsset->getFileName().getOriginal());
	return true;
}

bool HeightfieldPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	Ref< const HeightfieldAsset > heightfieldAsset = checked_type_cast< const HeightfieldAsset* >(sourceAsset);

	// Load heightfield from source file.
	Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), heightfieldAsset->getFileName().getOriginal());
	if (!file)
	{
		log::error << L"Heightfield pipeline failed; unable to open source (" << heightfieldAsset->getFileName().getOriginal() << L")" << Endl;
		return false;
	}

	Ref< Heightfield > heightfield = HeightfieldFormat().read(
		file,
		heightfieldAsset->getFileName().getExtension(),
		heightfieldAsset->getWorldExtent(),
		heightfieldAsset->getInvertX(),
		heightfieldAsset->getInvertZ(),
		heightfieldAsset->getDetailSkip()
	);
	if (!heightfield)
	{
		log::error << L"Heightfield pipeline failed; unable to read heightfield source \"" << heightfieldAsset->getFileName().getOriginal() << L"\"" << Endl;
		return 0;
	}

	// Create height field resource.
	Ref< HeightfieldResource > resource = new HeightfieldResource();

	// Create instance's name.
	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Heightfield pipeline failed; unable to create instance" << Endl;
		return false;
	}

	Ref< IStream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Heightfield pipeline failed; unable to create data stream" << Endl;
		instance->revert();
		return false;
	}

	const height_t* heights = heightfield->getHeights();
	uint32_t size = heightfield->getSize();

	Writer(stream).write(
		heights,
		size * size,
		sizeof(height_t)
	);

	stream->close();
	
	resource->m_size = size;
	resource->m_worldExtent = heightfieldAsset->getWorldExtent();

	instance->setObject(resource);

	return instance->commit();
}

Ref< ISerializable > HeightfieldPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
