#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyBoolean.h"
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
#include "Render/Shader.h"
#include "Resource/Id.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const resource::Id< render::Shader > c_idShaderSolid(Guid(L"{4F6F6CCE-97EC-624D-96B7-842F1D99D060}"));
const resource::Id< render::Shader > c_idShaderTextured(Guid(L"{049F4B08-1A54-DB4C-86CC-B533BCFFC65D}"));
const resource::Id< render::Shader > c_idShaderSolidCurve(Guid(L"{E942960D-81C2-FD4C-B005-009902CBD91E}"));
const resource::Id< render::Shader > c_idShaderTexturedCurve(Guid(L"{209E791F-C8E8-E646-973B-2910CC99C244}"));
const resource::Id< render::Shader > c_idShaderSolidMask(Guid(L"{D46877B9-0F90-3A42-AB2D-7346AA607233}"));
const resource::Id< render::Shader > c_idShaderTexturedMask(Guid(L"{5CDDBEC8-1629-0A4E-ACE5-C8186072D694}"));
const resource::Id< render::Shader > c_idShaderSolidMaskCurve(Guid(L"{BDC662CF-8A6B-BE42-BAEE-B12313EC3DDC}"));
const resource::Id< render::Shader > c_idShaderTexturedMaskCurve(Guid(L"{4FCA84E5-A055-BD48-8EAF-EB118B8C9BF7}"));
const resource::Id< render::Shader > c_idShaderIncrementMask(Guid(L"{8DCBCF05-4640-884E-95AC-F090510788F4}"));
const resource::Id< render::Shader > c_idShaderDecrementMask(Guid(L"{57F6F4DF-F4EE-6740-907C-027A3A2596D7}"));

const resource::Id< render::Shader > c_idShaderQuadSolid(Guid(L"{1EDAAA67-1E02-8A49-B857-14D7812C96D6}"));
const resource::Id< render::Shader > c_idShaderQuadTextured(Guid(L"{10426D17-CF0A-4849-A207-24F101A78459}"));
const resource::Id< render::Shader > c_idShaderQuadSolidMask(Guid(L"{2EDC5E1B-562D-9F46-9E3C-474729FB078E}"));
const resource::Id< render::Shader > c_idShaderQuadTexturedMask(Guid(L"{98A59F6A-1D90-144C-B688-4CEF382453F2}"));
const resource::Id< render::Shader > c_idShaderQuadIncrementMask(Guid(L"{16868DF6-A619-5541-83D2-94088A0AC552}"));
const resource::Id< render::Shader > c_idShaderQuadDecrementMask(Guid(L"{D6821007-47BB-D748-9E29-20829ED09C70}"));

const resource::Id< render::Shader > c_idShaderGlyph(Guid(L"{A8BC2D03-EB52-B744-8D4B-29E39FF0B4F5}"));
const resource::Id< render::Shader > c_idShaderGlyphMask(Guid(L"{C8FEF24B-D775-A14D-9FF3-E34A17495FB4}"));

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashPipeline", 21, FlashPipeline, editor::IPipeline)

FlashPipeline::FlashPipeline()
:	m_allowNPOT(true)
{
}

bool FlashPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	m_allowNPOT = settings->getProperty< PropertyBoolean >(L"FlashPipeline.AllowNPOT", true);
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
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const FlashMovieAsset* movieAsset = checked_type_cast< const FlashMovieAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), movieAsset->getFileName().getOriginal());

	// Add dependency to dependent flash movies.
	for (std::vector< Guid >::const_iterator i = movieAsset->m_dependentMovies.begin(); i != movieAsset->m_dependentMovies.end(); ++i)
		pipelineDepends->addDependency(*i, editor::PdfBuild);

	// AccShape
	pipelineDepends->addDependency(c_idShaderSolid, editor::PdfBuild | editor::PdfResource);	// Solid
	pipelineDepends->addDependency(c_idShaderTextured, editor::PdfBuild | editor::PdfResource);	// Textured
	pipelineDepends->addDependency(c_idShaderSolidCurve, editor::PdfBuild | editor::PdfResource);	// Solid Curve
	pipelineDepends->addDependency(c_idShaderTexturedCurve, editor::PdfBuild | editor::PdfResource);	// Textured Curve
	pipelineDepends->addDependency(c_idShaderSolidMask, editor::PdfBuild | editor::PdfResource);	// Solid Mask
	pipelineDepends->addDependency(c_idShaderTexturedMask, editor::PdfBuild | editor::PdfResource);	// Textured Mask
	pipelineDepends->addDependency(c_idShaderSolidMaskCurve, editor::PdfBuild | editor::PdfResource);	// Solid Mask Curve
	pipelineDepends->addDependency(c_idShaderTexturedMaskCurve, editor::PdfBuild | editor::PdfResource);	// Textured Mask Curve
	pipelineDepends->addDependency(c_idShaderIncrementMask, editor::PdfBuild | editor::PdfResource);	// Increment Mask
	pipelineDepends->addDependency(c_idShaderDecrementMask, editor::PdfBuild | editor::PdfResource);	// Decrement Mask

	// AccQuad
	pipelineDepends->addDependency(c_idShaderQuadSolid, editor::PdfBuild | editor::PdfResource);	// Solid Quad
	pipelineDepends->addDependency(c_idShaderQuadTextured, editor::PdfBuild | editor::PdfResource);	// Textured Quad
	pipelineDepends->addDependency(c_idShaderQuadSolidMask, editor::PdfBuild | editor::PdfResource);	// Solid Mask Quad
	pipelineDepends->addDependency(c_idShaderQuadTexturedMask, editor::PdfBuild | editor::PdfResource);	// Textured Mask Quad
	pipelineDepends->addDependency(c_idShaderQuadIncrementMask, editor::PdfBuild | editor::PdfResource);	// Increment Mask Quad
	pipelineDepends->addDependency(c_idShaderQuadDecrementMask, editor::PdfBuild | editor::PdfResource);	// Decrement Mask Quad

	// AccGlyph
	pipelineDepends->addDependency(c_idShaderGlyph, editor::PdfBuild | editor::PdfResource);	// Glyph
	pipelineDepends->addDependency(c_idShaderGlyphMask, editor::PdfBuild | editor::PdfResource);	// Glyph Mask

	return true;
}

bool FlashPipeline::buildOutput(
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
	const FlashMovieAsset* movieAsset = checked_type_cast< const FlashMovieAsset* >(sourceAsset);

	Ref< IStream > sourceStream = pipelineBuilder->openFile(Path(m_assetPath), movieAsset->getFileName().getOriginal());
	if (!sourceStream)
	{
		log::error << L"Failed to import Flash; unable to open file \"" << movieAsset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	Ref< SwfReader > swf = new SwfReader(sourceStream);
	Ref< FlashMovie > movie = flash::FlashMovieFactory(m_allowNPOT).createMovie(swf);
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

Ref< ISerializable > FlashPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
