/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <list>
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Shader.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Resource/Id.h"
#include "Spark/BitmapImage.h"
#include "Spark/BitmapResource.h"
#include "Spark/Frame.h"
#include "Spark/Movie.h"
#include "Spark/Optimizer.h"
#include "Spark/Packer.h"
#include "Spark/Sprite.h"
#include "Spark/Editor/ConvertImage.h"
#include "Spark/Editor/ConvertSvg.h"
#include "Spark/Editor/ConvertSwf.h"
#include "Spark/Editor/EmptyMovieAsset.h"
#include "Spark/Editor/MovieAsset.h"
#include "Spark/Editor/Pipeline.h"

namespace traktor::spark
{
	namespace
	{

const Guid c_idFlashShaderAssets(L"{14D6A2DB-796D-E54D-9D70-73DE4AE7C4E8}");

struct AtlasBitmap
{
	uint16_t id;
	Ref< const BitmapImage > bitmap;
	Packer::Rectangle packedRect;
};

struct AtlasBucket
{
	Ref< Packer > packer;
	std::list< AtlasBitmap > bitmaps;
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.Pipeline", 11, Pipeline, editor::IPipeline)

Pipeline::Pipeline()
:	m_generateMips(false)
,	m_sharpenStrength(0.0f)
,	m_useTextureCompression(true)
,	m_textureSizeDenom(1)
,	m_textureAtlasSize(1024)
{
}

bool Pipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	m_generateMips = settings->getPropertyIncludeHash< bool >(L"Pipeline.GenerateMips", false);
	m_sharpenStrength = settings->getPropertyIncludeHash< bool >(L"Pipeline.SharpenStrength", false);
	m_useTextureCompression = settings->getPropertyIncludeHash< bool >(L"Pipeline.UseTextureCompression", true);
	m_textureSizeDenom = settings->getPropertyIncludeHash< int32_t >(L"Pipeline.TextureSizeDenom", 1);
	m_textureAtlasSize = settings->getPropertyIncludeHash< int32_t >(L"Pipeline.TextureAtlasSize", 1024);
	return true;
}

void Pipeline::destroy()
{
}

TypeInfoSet Pipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		EmptyMovieAsset,
		MovieAsset
	>();
}

bool Pipeline::shouldCache() const
{
	return true;
}

uint32_t Pipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool Pipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const MovieAsset* movieAsset = dynamic_type_cast<const MovieAsset*>(sourceAsset))
	{
		pipelineDepends->addDependency(traktor::Path(m_assetPath), movieAsset->getFileName().getOriginal());
		for (const auto& font : movieAsset->getFonts())
			pipelineDepends->addDependency(traktor::Path(m_assetPath), font.fileName.getOriginal());
	}
	pipelineDepends->addDependency(c_idFlashShaderAssets, editor::PdfBuild | editor::PdfResource);
	pipelineDepends->addDependency< render::TextureOutput >();
	return true;
}

bool Pipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	Ref< Movie > movie;
	bool optimize = false;

	if (const MovieAsset* movieAsset = dynamic_type_cast< const MovieAsset* >(sourceAsset))
	{
		const traktor::Path filePath = FileSystem::getInstance().getAbsolutePath(traktor::Path(m_assetPath) + movieAsset->getFileName());
		Ref< IStream > sourceStream = FileSystem::getInstance().open(filePath, File::FmRead);
		if (!sourceStream)
		{
			log::error << L"Failed to import Spark movie; unable to open file \"" << movieAsset->getFileName().getOriginal() << L"\"." << Endl;
			return false;
		}

		const std::wstring extension = toLower(movieAsset->getFileName().getExtension());
		if (extension == L"swf")
			movie = convertSwf(sourceInstance, sourceStream);
		else if (extension == L"svg")
			movie = convertSvg(traktor::Path(m_assetPath), movieAsset, sourceInstance, sourceStream);
		else
			movie = convertImage(sourceInstance, sourceStream, extension);

		safeClose(sourceStream);

		optimize = movieAsset->m_staticMovie;
	}
	else if (const EmptyMovieAsset* emptyMovieAsset = dynamic_type_cast< const EmptyMovieAsset* >(sourceAsset))
	{
		const Color4ub& bc = emptyMovieAsset->getBackgroundColor();

		Ref< Sprite > sprite = new Sprite(emptyMovieAsset->getFrameRate());

		Ref< Frame > frame = new Frame();
		frame->changeBackgroundColor(Color4f(bc.r, bc.g, bc.b, bc.a) / Scalar(255.0f));
		sprite->addFrame(frame);

		movie = new Movie(
			Aabb2(
				Vector2(0.0f, 0.0f),
				Vector2(emptyMovieAsset->getStageWidth() * 20.0f, emptyMovieAsset->getStageHeight() * 20.0f)
			),
			sprite
		);
	}

	// Show some information about the Flash.
	log::info << L"Spark movie successfully loaded," << Endl;
	log::info << IncreaseIndent;
	log::info << movie->getFonts().size() << L" font(s)" << Endl;
	log::info << movie->getBitmaps().size() << L" bitmap(s)" << Endl;
	log::info << movie->getSounds().size() << L" sound(s)" << Endl;
	log::info << movie->getCharacters().size() << L" character(s)" << Endl;
	log::info << DecreaseIndent;

	// Merge all characters of first frame into a single sprite.
	if (optimize)
	{
		movie = Optimizer().merge(movie);
		if (!movie)
		{
			log::error << L"Failed to import Spark movie; failed to optimize static movie." << Endl;
			return false;
		}
	}

	// Generate triangles of every shape in movie.
	Optimizer().triangulate(movie, false);

	// Replace all bitmaps with resource references to textures.
	SmallMap< uint16_t, Ref< Bitmap > > bitmaps = movie->getBitmaps();

	// Create atlas buckets of small bitmaps.
	std::list< AtlasBucket > buckets;
	std::list< AtlasBitmap > standalone;
	Packer::Rectangle r;

	for (SmallMap< uint16_t, Ref< Bitmap > >::const_iterator i = bitmaps.begin(); i != bitmaps.end(); ++i)
	{
		const BitmapImage* bitmapData = dynamic_type_cast< const BitmapImage* >(i->second);
		if (!bitmapData)
		{
			log::warning << L"Skipped bitmap as it not a static bitmap (" << type_name(i->second) << L")" << Endl;
			continue;
		}

		bool foundBucket = false;
		for (auto& bucket : buckets)
		{
			if (bucket.packer->insert(bitmapData->getWidth() + 2, bitmapData->getHeight() + 2, r))
			{
				AtlasBitmap ab;
				ab.id = i->first;
				ab.bitmap = bitmapData;
				ab.packedRect = r;
				ab.packedRect.x += 1;
				ab.packedRect.y += 1;
				ab.packedRect.width = bitmapData->getWidth();
				ab.packedRect.height = bitmapData->getHeight();
				bucket.bitmaps.push_back(ab);
				foundBucket = true;
				break;
			}
		}
		if (!foundBucket)
		{
			buckets.push_back(AtlasBucket());

			AtlasBucket& b = buckets.back();
			b.packer = new Packer(m_textureAtlasSize, m_textureAtlasSize);

			if (b.packer->insert(bitmapData->getWidth() + 2, bitmapData->getHeight() + 2, r))
			{
				AtlasBitmap ab;
				ab.id = i->first;
				ab.bitmap = bitmapData;
				ab.packedRect = r;
				ab.packedRect.x += 1;
				ab.packedRect.y += 1;
				ab.packedRect.width = bitmapData->getWidth();
				ab.packedRect.height = bitmapData->getHeight();
				b.bitmaps.push_back(ab);
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

	for (const auto& bucket : buckets)
	{
		log::info << L"Atlas " << count << L", containing " << uint32_t(bucket.bitmaps.size()) << L" bitmaps." << Endl;

		if (bucket.bitmaps.size() > 1)
		{
			Ref< drawing::Image > atlasImage = new drawing::Image(
				drawing::PixelFormat::getA8B8G8R8(),
				m_textureAtlasSize,
				m_textureAtlasSize
			);

			atlasImage->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

			for (const auto& ab : bucket.bitmaps)
			{
				Ref< drawing::Image > bitmapImage = new drawing::Image(
					drawing::PixelFormat::getA8B8G8R8(),
					ab.bitmap->getWidth(),
					ab.bitmap->getHeight()
				);

				std::memcpy(
					bitmapImage->getData(),
					ab.bitmap->getBits(),
					ab.bitmap->getWidth() * ab.bitmap->getHeight() * 4
				);

				for (int32_t y = -1; y < ab.packedRect.height + 1; ++y)
				{
					for (int32_t x = -1; x < ab.packedRect.width + 1; ++x)
					{
						int32_t sx = x;
						int32_t sy = y;

						if (sx < 0)
							sx = ab.packedRect.width - 1;
						else if (sx > ab.packedRect.width - 1)
							sx = 0;

						if (sy < 0)
							sy = ab.packedRect.height - 1;
						else if (sy > ab.packedRect.height - 1)
							sy = 0;

						Color4f tmp;
						bitmapImage->getPixel(sx, sy, tmp);

						atlasImage->setPixel(ab.packedRect.x + x, ab.packedRect.y + y, tmp);
					}
				}
			}

#if defined(_DEBUG)
			atlasImage->save(L"SparkBitmapAtlas" + toString(count) + L".png");
#endif

			const Guid bitmapOutputGuid = outputGuid.permutation(count++);

			Ref< render::TextureOutput > output = new render::TextureOutput();
			output->m_textureFormat = render::TfInvalid;
			output->m_normalMap = false;
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
			output->m_inverseNormalMapY = false;
			output->m_assumeLinearGamma = true;
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

			const std::wstring bitmapOutputPath = traktor::Path(outputPath).getPathOnly() + L"/Textures/" + bitmapOutputGuid.format();
			if (!pipelineBuilder->buildAdHocOutput(
				output,
				bitmapOutputPath,
				bitmapOutputGuid,
				atlasImage
			))
				return false;

			for (const auto& ab : bucket.bitmaps)
			{
				movie->defineBitmap(ab.id, new BitmapResource(
					ab.packedRect.x,
					ab.packedRect.y,
					ab.packedRect.width,
					ab.packedRect.height,
					m_textureAtlasSize,
					m_textureAtlasSize,
					bitmapOutputGuid
				));
			}
		}
		else if (bucket.bitmaps.size() == 1)
		{
			AtlasBitmap ab = bucket.bitmaps.front();
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
		bitmapImage->save(L"SparkBitmap" + toString(count) + L".png");
#endif

		Guid bitmapOutputGuid = outputGuid.permutation(count++);

		Ref< render::TextureOutput > output = new render::TextureOutput();
		output->m_textureFormat = render::TfInvalid;
		output->m_normalMap = false;
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
		output->m_inverseNormalMapY = false;
		output->m_assumeLinearGamma = true;
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

		const std::wstring bitmapOutputPath = traktor::Path(outputPath).getPathOnly() + L"/Textures/" + bitmapOutputGuid.format();
		if (!pipelineBuilder->buildAdHocOutput(
			output,
			bitmapOutputPath,
			bitmapOutputGuid,
			bitmapImage
		))
			return false;

		movie->defineBitmap(i->id, new BitmapResource(
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
		log::error << L"Failed to import Spark movie; unable to create instance." << Endl;
		return false;
	}

	instance->setObject(movie);

	if (!instance->commit())
	{
		log::info << L"Failed to import Spark movie; unable to commit instance." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > Pipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

}
