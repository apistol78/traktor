#include "Flash/Editor/FlashPipeline.h"
#include "Flash/Editor/FlashMovieAsset.h"
#include "Flash/FlashMovieResource.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Database/Instance.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.flash.FlashPipeline", FlashPipeline, editor::IPipeline)

bool FlashPipeline::create(const editor::Settings* settings)
{
	return true;
}

void FlashPipeline::destroy()
{
}

uint32_t FlashPipeline::getVersion() const
{
	return 1;
}

TypeSet FlashPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< FlashMovieAsset >());
	return typeSet;
}

bool FlashPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const FlashMovieAsset* movieAsset = checked_type_cast< const FlashMovieAsset* >(sourceAsset);
	pipelineDepends->addDependency(movieAsset->getFileName());

	// Add dependencies to shaders.
	pipelineDepends->addDependency(Guid(L"{4F6F6CCE-97EC-624D-96B7-842F1D99D060}"), true);	// Solid
	pipelineDepends->addDependency(Guid(L"{049F4B08-1A54-DB4C-86CC-B533BCFFC65D}"), true);	// Textured
	pipelineDepends->addDependency(Guid(L"{D46877B9-0F90-3A42-AB2D-7346AA607233}"), true);	// Solid Mask
	pipelineDepends->addDependency(Guid(L"{5CDDBEC8-1629-0A4E-ACE5-C8186072D694}"), true);	// Textured Mask
	pipelineDepends->addDependency(Guid(L"{8DCBCF05-4640-884E-95AC-F090510788F4}"), true);	// Increment Mask
	pipelineDepends->addDependency(Guid(L"{57F6F4DF-F4EE-6740-907C-027A3A2596D7}"), true);	// Decrement Mask

	return true;
}

bool FlashPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const Serializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const FlashMovieAsset* movieAsset = checked_type_cast< const FlashMovieAsset* >(sourceAsset);

	Ref< Stream > sourceStream = FileSystem::getInstance().open(movieAsset->getFileName(), File::FmRead);
	if (!sourceStream)
	{
		log::error << L"Failed to import flash, unable to open source" << Endl;
		return false;
	}

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Failed to build flash resource, unable to create instance" << Endl;
		return false;
	}

	instance->setObject(gc_new< flash::FlashMovieResource >());

	Ref< Stream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Failed to build flash resource, unable to create data stream" << Endl;
		instance->revert();
		return false;
	}

	uint8_t buf[1024];
	while (sourceStream->available() > 0)
	{
		int nread = sourceStream->read(buf, sizeof(buf));
		stream->write(buf, nread);
	}

	stream->close();
	sourceStream->close();

	if (!instance->commit())
	{
		log::info << L"Failed to build flash resource, unable to commit instance" << Endl;
		return false;
	}

	return true;
}

	}
}
