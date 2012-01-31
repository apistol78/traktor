#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/SwfReader.h"
#include "Flash/Editor/FlashPipeline.h"
#include "Flash/Editor/FlashMovieAsset.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashPipeline", 5, FlashPipeline, editor::IPipeline)

bool FlashPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
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
	pipelineDepends->addDependency(Guid(L"{1EDAAA67-1E02-8A49-B857-14D7812C96D6}"), editor::PdfBuild);	// Solid Quad
	pipelineDepends->addDependency(Guid(L"{10426D17-CF0A-4849-A207-24F101A78459}"), editor::PdfBuild);	// Textured Quad
	pipelineDepends->addDependency(Guid(L"{2EDC5E1B-562D-9F46-9E3C-474729FB078E}"), editor::PdfBuild);	// Solid Mask Quad
	pipelineDepends->addDependency(Guid(L"{98A59F6A-1D90-144C-B688-4CEF382453F2}"), editor::PdfBuild);	// Textured Mask Quad
	pipelineDepends->addDependency(Guid(L"{A8BC2D03-EB52-B744-8D4B-29E39FF0B4F5}"), editor::PdfBuild);	// Glyph
	pipelineDepends->addDependency(Guid(L"{C8FEF24B-D775-A14D-9FF3-E34A17495FB4}"), editor::PdfBuild);	// Glyph Mask

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
		log::error << L"Failed to import Flash; unable to open file \"" << fileName.getPathName() << L"\"" << Endl;
		return false;
	}

	Ref< SwfReader > swf = new SwfReader(sourceStream);
	Ref< FlashMovie > movie = flash::FlashMovieFactory().createMovie(swf);
	if (!movie)
	{
		log::error << L"Failed to import Flash; unable to parse SWF" << Endl;
		return false;
	}

	sourceStream->close();
	sourceStream = 0;


	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Failed to import Flash; unable to create instance" << Endl;
		return false;
	}

	instance->setObject(movie);

	if (!instance->commit())
	{
		log::info << L"Failed to import Flash; unable to commit instance" << Endl;
		return false;
	}

	return true;
}

	}
}
