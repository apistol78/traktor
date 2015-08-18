#include <cstring>
#include <list>
#include <MaxRectsBinPack.h>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Flash/FlashBitmapData.h"
#include "Flash/FlashBitmapResource.h"
#include "Flash/FlashFont.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/SwfReader.h"
#include "Flash/Editor/FlashPipeline.h"
#include "Flash/Editor/FlashMovieAsset.h"
#include "Render/Shader.h"
#include "Render/Editor/Texture/TextureOutput.h"
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

struct AtlasBitmap
{
	uint16_t id;
	Ref< const FlashBitmapData > bitmap;
	rbp::Rect packedRect;
};

struct AtlasBucket
{
	rbp::MaxRectsBinPack binPack;
	std::list< AtlasBitmap > bitmaps;
};

Guid incrementGuid(const Guid& g, uint32_t steps)
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = g[i];
	reinterpret_cast< uint32_t& >(d[12]) += steps;
	return Guid(d);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashPipeline", 37, FlashPipeline, editor::IPipeline)

FlashPipeline::FlashPipeline()
:	m_generateMips(false)
,	m_sharpenStrength(0.0f)
,	m_useTextureCompression(true)
,	m_textureSizeDenom(1)
{
}

bool FlashPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	m_generateMips = settings->getProperty< PropertyBoolean >(L"FlashPipeline.GenerateMips", false);
	m_sharpenStrength = settings->getProperty< PropertyBoolean >(L"FlashPipeline.SharpenStrength", false);
	m_useTextureCompression = settings->getProperty< PropertyBoolean >(L"FlashPipeline.UseTextureCompression", true);
	m_textureSizeDenom = settings->getProperty< PropertyInteger >(L"FlashPipeline.TextureSizeDenom", 1);
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
	Ref< FlashMovie > movie = flash::FlashMovieFactory().createMovie(swf);
	if (!movie)
	{
		log::error << L"Failed to import Flash; unable to parse SWF" << Endl;
		return false;
	}

	sourceStream->close();
	sourceStream = 0;

	// Show some information about the Flash.
	log::info << L"SWF successfully loaded," << Endl;
	log::info << IncreaseIndent;
	
	const SmallMap< uint16_t, Ref< FlashFont > >& fonts = movie->getFonts();
	log::info << fonts.size() << L" font(s)" << Endl;
	log::info << IncreaseIndent;
	for (SmallMap< uint16_t, Ref< FlashFont > >::const_iterator i = fonts.begin(); i != fonts.end(); ++i)
		log::info << i->first << L". " << i->second->getShapes().size() << L" glyph(s)" << Endl;
	log::info << DecreaseIndent;

	log::info << movie->getBitmaps().size() << L" bitmap(s)" << Endl;
	log::info << movie->getSounds().size() << L" sound(s)" << Endl;
	log::info << movie->getCharacters().size() << L" character(s)" << Endl;

	log::info << DecreaseIndent;

	// Replace all bitmaps with resource references to textures.
	SmallMap< uint16_t, Ref< FlashBitmap > > bitmaps = movie->getBitmaps();

	// Create atlas buckets of small bitmaps.
	std::list< AtlasBucket > buckets;
	std::list< AtlasBitmap > standalone;

	for (SmallMap< uint16_t, Ref< FlashBitmap > >::const_iterator i = bitmaps.begin(); i != bitmaps.end(); ++i)
	{
		const FlashBitmapData* bitmapData = dynamic_type_cast< const FlashBitmapData* >(i->second);
		if (!bitmapData)
		{
			log::warning << L"Skipped bitmap as it not a static bitmap (" << type_name(i->second) << L")" << Endl;
			continue;
		}

		bool foundBucket = false;

		for (std::list< AtlasBucket >::iterator j = buckets.begin(); j != buckets.end(); ++j)
		{
			int32_t mdim = std::max(bitmapData->getWidth(), bitmapData->getHeight());
			rbp::Rect packedRect = j->binPack.Insert(
				mdim + 2,
				mdim + 2,
				rbp::MaxRectsBinPack::RectBestAreaFit
			);
			if (packedRect.height > 0)
			{
				AtlasBitmap ab;
				ab.id = i->first;
				ab.bitmap = bitmapData;
				ab.packedRect = packedRect;
				ab.packedRect.x += 1;
				ab.packedRect.y += 1;
				ab.packedRect.width = bitmapData->getWidth();
				ab.packedRect.height = bitmapData->getHeight();
				j->bitmaps.push_back(ab);
				foundBucket = true;
				break;
			}
		}

		if (!foundBucket)
		{
			buckets.push_back(AtlasBucket());
			buckets.back().binPack.Init(1024, 1024);

			int32_t mdim = std::max(bitmapData->getWidth(), bitmapData->getHeight());
			rbp::Rect packedRect = buckets.back().binPack.Insert(
				mdim + 2,
				mdim + 2,
				rbp::MaxRectsBinPack::RectBestAreaFit
			);
			if (packedRect.height > 0)
			{
				AtlasBitmap ab;
				ab.id = i->first;
				ab.bitmap = bitmapData;
				ab.packedRect = packedRect;
				ab.packedRect.x += 1;
				ab.packedRect.y += 1;
				ab.packedRect.width = bitmapData->getWidth();
				ab.packedRect.height = bitmapData->getHeight();
				buckets.back().bitmaps.push_back(ab);
			}
			else
			{
				AtlasBitmap ab;
				ab.id = i->first;
				ab.bitmap = bitmapData;
				ab.packedRect.x = 0;
				ab.packedRect.y = 0;
				ab.packedRect.width = bitmapData->getWidth();
				ab.packedRect.height = bitmapData->getHeight();
				standalone.push_back(ab);
			}
		}
	}

	log::info << L"Packed bitmaps into " << uint32_t(buckets.size()) << L" atlas(es)." << Endl;

	uint32_t count = 1;

	for (std::list< AtlasBucket >::const_iterator i = buckets.begin(); i != buckets.end(); ++i)
	{
		log::info << L"Atlas " << count << L", containing " << uint32_t(i->bitmaps.size()) << L" bitmaps." << Endl;

		if (i->bitmaps.size() > 1)
		{
			Ref< drawing::Image > atlasImage = new drawing::Image(
				drawing::PixelFormat::getA8B8G8R8(),
				1024,
				1024
			);

			atlasImage->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

			for (std::list< AtlasBitmap >::const_iterator j = i->bitmaps.begin(); j != i->bitmaps.end(); ++j)
			{
				Ref< drawing::Image > bitmapImage = new drawing::Image(
					drawing::PixelFormat::getA8B8G8R8(),
					j->bitmap->getWidth(),
					j->bitmap->getHeight()
				);

				std::memcpy(
					bitmapImage->getData(),
					j->bitmap->getBits(),
					j->bitmap->getWidth() * j->bitmap->getHeight() * 4
				);

				for (int32_t y = -1; y < j->packedRect.height + 1; ++y)
				{
					for (int32_t x = -1; x < j->packedRect.width + 1; ++x)
					{
						int32_t sx = x;
						int32_t sy = y;

						if (sx < 0)
							sx = j->packedRect.width - 1;
						else if (sx > j->packedRect.width - 1)
							sx = 0;

						if (sy < 0)
							sy = j->packedRect.height - 1;
						else if (sy > j->packedRect.height - 1)
							sy = 0;

						Color4f tmp;
						bitmapImage->getPixel(sx, sy, tmp);

						atlasImage->setPixel(j->packedRect.x + x, j->packedRect.y + y, tmp);
					}
				}
			}

#if defined(_DEBUG)
			atlasImage->save(L"FlashBitmapAtlas" + toString(count) + L".png");
#endif

			Guid bitmapOutputGuid = incrementGuid(outputGuid, count++);

			Ref< render::TextureOutput > output = new render::TextureOutput();
			output->m_textureFormat = render::TfInvalid;
			output->m_generateNormalMap = false;
			output->m_scaleDepth = 0.0f;
			output->m_generateMips = m_generateMips;
			output->m_keepZeroAlpha = false;
			output->m_textureType = render::Tt2D;
			output->m_hasAlpha = false;
			output->m_ignoreAlpha = false;
			output->m_scaleImage = false;
			output->m_scaleWidth = 0;
			output->m_scaleHeight = 0;
			output->m_enableCompression = m_useTextureCompression;
			output->m_enableNormalMapCompression = false;
			output->m_inverseNormalMapY = false;
			output->m_linearGamma = true;
			output->m_generateSphereMap = false;
			output->m_preserveAlphaCoverage = false;
			output->m_alphaCoverageReference = 0.0f;
			output->m_sharpenRadius = m_sharpenStrength > 0.0f ? 5 : 0;
			output->m_sharpenStrength = m_sharpenStrength;
			output->m_systemTexture = true;

			if (m_textureSizeDenom > 1)
			{
				output->m_scaleImage = true;
				output->m_scaleWidth = atlasImage->getWidth() / m_textureSizeDenom;
				output->m_scaleHeight = atlasImage->getHeight() / m_textureSizeDenom;
			}

			std::wstring bitmapOutputPath = Path(outputPath).getPathOnly() + L"/Textures/" + bitmapOutputGuid.format();
			if (!pipelineBuilder->buildOutput(
				output,
				bitmapOutputPath,
				bitmapOutputGuid,
				atlasImage
			))
				return false;

			for (std::list< AtlasBitmap >::const_iterator j = i->bitmaps.begin(); j != i->bitmaps.end(); ++j)
			{
				movie->defineBitmap(j->id, new FlashBitmapResource(
					j->packedRect.x,
					j->packedRect.y,
					j->packedRect.width,
					j->packedRect.height,
					1024,
					1024,
					bitmapOutputGuid
				));
			}
		}
		else if (i->bitmaps.size() == 1)
		{
			AtlasBitmap ab = i->bitmaps.front();
			ab.packedRect.x = 0;
			ab.packedRect.y = 0;
			ab.packedRect.width = ab.bitmap->getWidth();
			ab.packedRect.height = ab.bitmap->getHeight();
			standalone.push_back(ab);
		}
	}

	log::info << uint32_t(standalone.size()) << L" bitmap(s) didn't fit in any atlas..." << Endl;

	for (std::list< AtlasBitmap >::const_iterator i = standalone.begin(); i != standalone.end(); ++i)
	{
		Ref< drawing::Image > bitmapImage = new drawing::Image(
			drawing::PixelFormat::getA8B8G8R8(),
			i->bitmap->getWidth(),
			i->bitmap->getHeight()
		);

		std::memcpy(
			bitmapImage->getData(),
			i->bitmap->getBits(),
			i->bitmap->getWidth() * i->bitmap->getHeight() * 4
		);

#if defined(_DEBUG)
		bitmapImage->save(L"FlashBitmap" + toString(count) + L".png");
#endif

		Guid bitmapOutputGuid = incrementGuid(outputGuid, count++);

		Ref< render::TextureOutput > output = new render::TextureOutput();
		output->m_textureFormat = render::TfInvalid;
		output->m_generateNormalMap = false;
		output->m_scaleDepth = 0.0f;
		output->m_generateMips = m_generateMips;
		output->m_keepZeroAlpha = false;
		output->m_textureType = render::Tt2D;
		output->m_hasAlpha = false;
		output->m_ignoreAlpha = false;
		output->m_scaleImage = false;
		output->m_scaleWidth = 0;
		output->m_scaleHeight = 0;
		output->m_enableCompression = m_useTextureCompression;
		output->m_enableNormalMapCompression = false;
		output->m_inverseNormalMapY = false;
		output->m_linearGamma = true;
		output->m_generateSphereMap = false;
		output->m_preserveAlphaCoverage = false;
		output->m_alphaCoverageReference = 0.0f;
		output->m_sharpenRadius = m_sharpenStrength > 0.0f ? 5 : 0;
		output->m_sharpenStrength = m_sharpenStrength;
		output->m_systemTexture = true;

		if (m_textureSizeDenom > 1)
		{
			output->m_scaleImage = true;
			output->m_scaleWidth = bitmapImage->getWidth() / m_textureSizeDenom;
			output->m_scaleHeight = bitmapImage->getHeight() / m_textureSizeDenom;
		}

		std::wstring bitmapOutputPath = Path(outputPath).getPathOnly() + L"/Textures/" + bitmapOutputGuid.format();
		if (!pipelineBuilder->buildOutput(
			output,
			bitmapOutputPath,
			bitmapOutputGuid,
			bitmapImage
		))
			return false;

		movie->defineBitmap(i->id, new FlashBitmapResource(
			0,
			0,
			bitmapImage->getWidth(),
			bitmapImage->getHeight(),
			bitmapImage->getWidth(),
			bitmapImage->getHeight(),
			bitmapOutputGuid
		));
	}

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
