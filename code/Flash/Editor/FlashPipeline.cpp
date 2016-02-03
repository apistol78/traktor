#include <cstring>
#include <list>
#include <MaxRectsBinPack.h>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
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
#include "Flash/FlashFrame.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/FlashOptimizer.h"
#include "Flash/FlashSprite.h"
#include "Flash/SwfReader.h"
#include "Flash/Editor/FlashEmptyMovieAsset.h"
#include "Flash/Editor/FlashMovieAsset.h"
#include "Flash/Editor/FlashPipeline.h"
#include "Render/Shader.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Resource/Id.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const Guid c_idFlashShaderAssets(L"{14D6A2DB-796D-E54D-9D70-73DE4AE7C4E8}");

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

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashPipeline", 43, FlashPipeline, editor::IPipeline)

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
	typeSet.insert(&type_of< FlashEmptyMovieAsset >());
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
	if (const FlashMovieAsset* movieAsset = dynamic_type_cast< const FlashMovieAsset* >(sourceAsset))
		pipelineDepends->addDependency(Path(m_assetPath), movieAsset->getFileName().getOriginal());
	pipelineDepends->addDependency(c_idFlashShaderAssets, editor::PdfBuild | editor::PdfResource);	// Solid
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
	Ref< FlashMovie > movie;
	bool optimize = false;

	if (const FlashMovieAsset* movieAsset = dynamic_type_cast< const FlashMovieAsset* >(sourceAsset))
	{
		Ref< IStream > sourceStream = pipelineBuilder->openFile(Path(m_assetPath), movieAsset->getFileName().getOriginal());
		if (!sourceStream)
		{
			log::error << L"Failed to import Flash; unable to open file \"" << movieAsset->getFileName().getOriginal() << L"\"" << Endl;
			return false;
		}

		Ref< SwfReader > swf = new SwfReader(sourceStream);
		movie = FlashMovieFactory(true).createMovie(swf);
		if (!movie)
		{
			log::error << L"Failed to import Flash; unable to parse SWF" << Endl;
			return false;
		}

		safeClose(sourceStream);

		optimize = movieAsset->m_staticMovie;
	}
	else if (const FlashEmptyMovieAsset* emptyMovieAsset = dynamic_type_cast< const FlashEmptyMovieAsset* >(sourceAsset))
	{
		const Color4ub& bc = emptyMovieAsset->getBackgroundColor();
		SwfColor backgroundColor = { bc.r, bc.g, bc.b, bc.a };
		Ref< FlashSprite > sprite = new FlashSprite(0, emptyMovieAsset->getFrameRate());

		Ref< FlashFrame > frame = new FlashFrame();
		frame->changeBackgroundColor(backgroundColor);
		sprite->addFrame(frame);

		movie = new FlashMovie(
			Aabb2(
				Vector2(0.0f, 0.0f),
				Vector2(emptyMovieAsset->getStageWidth() * 20.0f, emptyMovieAsset->getStageHeight() * 20.0f)
			),
			sprite
		);
	}

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

	// Merge all characters of first frame into a single sprite.
	if (optimize)
	{
		movie = FlashOptimizer().merge(movie);
		if (!movie)
		{
			log::error << L"Failed to import Flash; failed to optimize static SWF" << Endl;
			return false;
		}
	}

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

			Guid bitmapOutputGuid = outputGuid.permutate(count++);

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

		Guid bitmapOutputGuid = outputGuid.permutate(count++);

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
