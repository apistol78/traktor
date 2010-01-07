#include "Flash/Editor/FlashPipeline.h"
#include "Flash/Editor/FlashMovieAsset.h"
#include "Flash/FlashMovieResource.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineSettings.h"
#include "Database/Instance.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashPipeline", 2, FlashPipeline, editor::IPipeline)

bool FlashPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< editor::PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void FlashPipeline::destroy()
{
}

TypeInfoSet FlashPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< FlashMovieAsset >());
	return typeSet;
}

bool FlashPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const FlashMovieAsset* movieAsset = checked_type_cast< const FlashMovieAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, movieAsset->getFileName());
	pipelineDepends->addDependency(fileName);

	// Add dependencies to shaders.
	pipelineDepends->addDependency(Guid(L"{4F6F6CCE-97EC-624D-96B7-842F1D99D060}"), editor::PdfBuild);	// Solid
	pipelineDepends->addDependency(Guid(L"{049F4B08-1A54-DB4C-86CC-B533BCFFC65D}"), editor::PdfBuild);	// Textured
	pipelineDepends->addDependency(Guid(L"{D46877B9-0F90-3A42-AB2D-7346AA607233}"), editor::PdfBuild);	// Solid Mask
	pipelineDepends->addDependency(Guid(L"{5CDDBEC8-1629-0A4E-ACE5-C8186072D694}"), editor::PdfBuild);	// Textured Mask
	pipelineDepends->addDependency(Guid(L"{8DCBCF05-4640-884E-95AC-F090510788F4}"), editor::PdfBuild);	// Increment Mask
	pipelineDepends->addDependency(Guid(L"{57F6F4DF-F4EE-6740-907C-027A3A2596D7}"), editor::PdfBuild);	// Decrement Mask
	pipelineDepends->addDependency(Guid(L"{10426D17-CF0A-4849-A207-24F101A78459}"), editor::PdfBuild);	// Textured Quad

	return true;
}

bool FlashPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const FlashMovieAsset* movieAsset = checked_type_cast< const FlashMovieAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, movieAsset->getFileName());

	Ref< IStream > sourceStream = FileSystem::getInstance().open(fileName, File::FmRead);
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

	instance->setObject(new flash::FlashMovieResource());

	Ref< IStream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Failed to build flash resource, unable to create data stream" << Endl;
		instance->revert();
		return false;
	}

	StreamCopy(stream, sourceStream).execute();

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
