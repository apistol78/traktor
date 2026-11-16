/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <limits>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldTextureAsset.h"
#include "Heightfield/Editor/HeightfieldTileAsset.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Terrain/TerrainResource.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "Terrain/Editor/TerrainImageAsset.h"
#include "Terrain/Editor/TerrainPipeline.h"

namespace traktor::terrain
{
	namespace
	{

const Guid c_guidSplatMapSeed(L"{3BDF1640-E844-404b-85F2-277C022B8E23}");

const Guid c_guidTerrainShaderSeed(L"{6643B92A-6676-41b9-9427-3569B2EA481B}");
const Guid c_guidSurfaceShaderSeed(L"{8481FC82-A8E8-49b8-906F-9F8F6365B1F5}");
const Guid c_guidTerrainShaderTemplate(L"{A6C4532A-0540-4D42-93FC-964C7BFDD1FD}");
const Guid c_guidSurfaceShaderTemplate(L"{BAD675B3-9799-7D49-A045-BDA471DD5A3E}");

const Guid c_materialInterface(L"{139CACBD-2A79-5644-B9BC-B113F66D50EA}");

const Guid c_guidTileHeightfieldSeed(L"{DB111426-6ECE-4C29-BE4B-732A81D6246A}");
const Guid c_guidTileHeightMapSeed(L"{3427B3E2-E59F-4C2E-B715-D99469F778E2}");
const Guid c_guidTileNormalMapSeed(L"{073DC7E4-5FBE-498B-8CA2-6343F1C18B58}");
const Guid c_guidTileCutMapSeed(L"{9AD3F604-0541-414A-A0E9-E2AA767BD452}");
const Guid c_guidTileSplatMapSeed(L"{071FD99B-6D7E-4601-8DD2-C6C501B36572}");
const Guid c_guidTileColorMapSeed(L"{B9AB3117-689B-4282-92AB-42BF38E2C1E5}");


class FragmentReaderAdapter : public render::FragmentLinker::FragmentReaderNoCache
{
public:
	explicit FragmentReaderAdapter(editor::IPipelineBuilder* pipelineBuilder, const render::ShaderGraph* surfaceShaderImpl)
	:	m_pipelineBuilder(pipelineBuilder)
	,	m_surfaceShaderImpl(surfaceShaderImpl)
	{
	}

	virtual Ref< const render::ShaderGraph > read(const Guid& fragmentGuid) const
	{
		if (fragmentGuid == c_materialInterface)
			return m_surfaceShaderImpl;
		else
			return m_pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(fragmentGuid);
	}

private:
	Ref< editor::IPipelineBuilder > m_pipelineBuilder;
	Ref< const render::ShaderGraph > m_surfaceShaderImpl;
};

Guid combineGuids(const Guid& g1, const Guid& g2)
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = g1[i] ^ g2[i];
	return Guid(d);
}

//! Id of a tile resource; tile -1, -1 is the overview.
Guid tileGuid(const Guid& seed, const Guid& outputGuid, int32_t tileX, int32_t tileZ)
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = seed[i] ^ outputGuid[i];
	d[0] ^= (uint8_t)tileX;
	d[1] ^= (uint8_t)(tileX >> 8);
	d[2] ^= (uint8_t)tileZ;
	d[3] ^= (uint8_t)(tileZ >> 8);
	return Guid(d);
}

Ref< render::TextureOutput > createImageOutput()
{
	Ref< render::TextureOutput > output = new render::TextureOutput();
	output->m_keepZeroAlpha = false;
	output->m_ignoreAlpha = false;
	output->m_hasAlpha = true;
	output->m_assumeLinearGamma = true;
	output->m_systemTexture = true;
	return output;
}

//! Resample image with first and last texel centered on image's first and last pixel.
Ref< drawing::Image > resampleEdgeAligned(const drawing::Image* image, int32_t width, int32_t height)
{
	Ref< drawing::Image > output = new drawing::Image(image->getPixelFormat(), width, height);
	const float sx = (float)(image->getWidth() - 1) / (width - 1);
	const float sy = (float)(image->getHeight() - 1) / (height - 1);

	// Rows are resampled in parallel.
	const int32_t c_rowsPerTask = 64;
	AlignedVector< Job::task_t > tasks;
	for (int32_t y0 = 0; y0 < height; y0 += c_rowsPerTask)
	{
		tasks.push_back([&, y0]() {
			for (int32_t y = y0; y < std::min(y0 + c_rowsPerTask, height); ++y)
			{
				const int32_t iy = std::min((int32_t)(y * sy), (int32_t)image->getHeight() - 2);
				const Scalar fy(y * sy - iy);

				for (int32_t x = 0; x < width; ++x)
				{
					const int32_t ix = std::min((int32_t)(x * sx), (int32_t)image->getWidth() - 2);
					const Scalar fx(x * sx - ix);

					Color4f c00, c10, c01, c11;
					image->getPixelUnsafe(ix, iy, c00);
					image->getPixelUnsafe(ix + 1, iy, c10);
					image->getPixelUnsafe(ix, iy + 1, c01);
					image->getPixelUnsafe(ix + 1, iy + 1, c11);

					const Color4f c0 = c00 + (c10 - c00) * fx;
					const Color4f c1 = c01 + (c11 - c01) * fx;
					output->setPixelUnsafe(x, y, c0 + (c1 - c0) * fy);
				}
			}
		});
	}
	JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

	return output;
}

//! Add dependencies to resources of a tile, or overview if tile is -1, -1.
void addTileDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* terrainInstance,
	const db::Instance* heightfieldInstance,
	const hf::HeightfieldAsset* heightfieldAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	int32_t tileX,
	int32_t tileZ,
	int32_t overviewSize
)
{
	const uint32_t flags = editor::PdfBuild | editor::PdfResource;
	const int32_t tileCount = std::max(heightfieldAsset->getTileCount(), 1);
	const hf::TileStorage tileStorage(heightfieldInstance, heightfieldAsset->getTileCount());
	const std::wstring path = (tileX >= 0) ? outputPath + L"/Tiles/" + toString(tileX) + L"_" + toString(tileZ) : outputPath + L"/Overview";
	const uint32_t assetHash = DeepHash(heightfieldAsset).get();

	// Tiles read one row and column of their +X/+Z neighbours, normals also a border of samples; overview read every tile.
	const int32_t x0 = (tileX >= 0) ? tileX : 0;
	const int32_t z0 = (tileX >= 0) ? tileZ : 0;
	const int32_t x1 = (tileX >= 0) ? tileX + 1 : tileCount - 1;
	const int32_t z1 = (tileX >= 0) ? tileZ + 1 : tileCount - 1;
	const uint32_t stamp = assetHash + tileStorage.readStamp(L"Data", x0, z0, x1, z1);
	const uint32_t normalStamp = (tileX >= 0) ? assetHash + tileStorage.readStamp(L"Data", tileX - 1, tileZ - 1, tileX + 1, tileZ + 1) : stamp;

	Ref< hf::HeightfieldTileAsset > heightfieldTile = new hf::HeightfieldTileAsset();
	heightfieldTile->m_heightfield = resource::Id< hf::Heightfield >(heightfieldInstance->getGuid());
	heightfieldTile->m_tileX = tileX;
	heightfieldTile->m_tileZ = tileZ;
	heightfieldTile->m_overviewSize = overviewSize;
	heightfieldTile->m_stamp = stamp;
	pipelineDepends->addDependency(heightfieldTile, path + L"/Heightfield", tileGuid(c_guidTileHeightfieldSeed, outputGuid, tileX, tileZ), flags);

	const hf::HeightfieldTextureAsset::OutputType outputs[] = { hf::HeightfieldTextureAsset::OtHeights, hf::HeightfieldTextureAsset::OtNormals, hf::HeightfieldTextureAsset::OtCuts };
	const Guid outputSeeds[] = { c_guidTileHeightMapSeed, c_guidTileNormalMapSeed, c_guidTileCutMapSeed };
	const wchar_t* outputNames[] = { L"/Heights", L"/Normals", L"/Cuts" };

	for (int32_t i = 0; i < sizeof_array(outputs); ++i)
	{
		Ref< hf::HeightfieldTextureAsset > textureAsset = new hf::HeightfieldTextureAsset();
		textureAsset->m_heightfield = resource::Id< hf::Heightfield >(heightfieldInstance->getGuid());
		textureAsset->m_output = outputs[i];
		textureAsset->m_tileX = tileX;
		textureAsset->m_tileZ = tileZ;
		textureAsset->m_overviewSize = overviewSize;
		textureAsset->m_stamp = (outputs[i] == hf::HeightfieldTextureAsset::OtNormals) ? normalStamp : stamp;
		pipelineDepends->addDependency(textureAsset, path + outputNames[i], tileGuid(outputSeeds[i], outputGuid, tileX, tileZ), flags);
	}

	const wchar_t* images[] = { L"Splat", L"Color" };
	const Guid imageSeeds[] = { c_guidTileSplatMapSeed, c_guidTileColorMapSeed };
	const wchar_t* imageNames[] = { L"/Splat", L"/Colors" };

	for (int32_t i = 0; i < sizeof_array(images); ++i)
	{
		const hf::TileStorage terrainTileStorage(terrainInstance, heightfieldAsset->getTileCount());
		if (!terrainTileStorage.haveData(images[i]))
			continue;

		Ref< TerrainImageAsset > imageAsset = new TerrainImageAsset();
		imageAsset->m_instance = terrainInstance->getGuid();
		imageAsset->m_name = images[i];
		imageAsset->m_tileCount = heightfieldAsset->getTileCount();
		imageAsset->m_tileX = tileX;
		imageAsset->m_tileZ = tileZ;
		imageAsset->m_overviewSize = overviewSize;
		imageAsset->m_stamp = assetHash + terrainTileStorage.readStamp(images[i], x0, z0, x1, z1);
		pipelineDepends->addDependency(imageAsset, path + imageNames[i], tileGuid(imageSeeds[i], outputGuid, tileX, tileZ), flags);
	}
}

//! Resources of a tile, or overview if tile is -1, -1; tiles without splat image use default splat map.
TerrainResource::Tile createTile(const Guid& outputGuid, int32_t tileX, int32_t tileZ, bool haveSplat, bool haveColor, const Guid& defaultSplatMapGuid)
{
	TerrainResource::Tile tile;
	tile.heightfield = resource::Id< hf::Heightfield >(tileGuid(c_guidTileHeightfieldSeed, outputGuid, tileX, tileZ));
	tile.heightMap = resource::Id< render::ITexture >(tileGuid(c_guidTileHeightMapSeed, outputGuid, tileX, tileZ));
	tile.normalMap = resource::Id< render::ITexture >(tileGuid(c_guidTileNormalMapSeed, outputGuid, tileX, tileZ));
	tile.cutMap = resource::Id< render::ITexture >(tileGuid(c_guidTileCutMapSeed, outputGuid, tileX, tileZ));
	tile.splatMap = resource::Id< render::ITexture >(haveSplat ? tileGuid(c_guidTileSplatMapSeed, outputGuid, tileX, tileZ) : defaultSplatMapGuid);
	if (haveColor)
		tile.colorMap = resource::Id< render::ITexture >(tileGuid(c_guidTileColorMapSeed, outputGuid, tileX, tileZ));
	return tile;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainPipeline", 19, TerrainPipeline, editor::DefaultPipeline)

bool TerrainPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	return editor::DefaultPipeline::create(settings, database);
}

TypeInfoSet TerrainPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< TerrainAsset, TerrainImageAsset >();
}

bool TerrainPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (is_a< TerrainImageAsset >(sourceAsset))
	{
		pipelineDepends->addDependency< render::TextureOutput >();
		return true;
	}

	const TerrainAsset* terrainAsset = checked_type_cast< const TerrainAsset*, false >(sourceAsset);

	pipelineDepends->addDependency(terrainAsset->getHeightfield(), editor::PdfUse);
	pipelineDepends->addDependency(terrainAsset->getSurfaceShader(), editor::PdfUse);

	pipelineDepends->addDependency(c_guidTerrainShaderTemplate, editor::PdfUse);
	pipelineDepends->addDependency(c_guidSurfaceShaderTemplate, editor::PdfUse);

	pipelineDepends->addDependency(c_materialInterface, editor::PdfUse);

	// Resources for each tile, untiled terrains are a single tile, and a low resolution overview of entire terrain.
	Ref< const hf::HeightfieldAsset > heightfieldAsset = pipelineDepends->getObjectReadOnly< hf::HeightfieldAsset >(terrainAsset->getHeightfield());
	if (heightfieldAsset)
	{
		const int32_t tileCount = std::max(heightfieldAsset->getTileCount(), 1);

		Ref< db::Instance > heightfieldInstance = pipelineDepends->getSourceDatabase()->getInstance(terrainAsset->getHeightfield());
		const int32_t size = heightfieldInstance ? hf::TileStorage(heightfieldInstance, heightfieldAsset->getTileCount()).readSize() : 0;
		if (size <= 0)
		{
			log::error << L"Terrain pipeline failed; unable to read size of heightfield." << Endl;
			return false;
		}

		for (int32_t tz = 0; tz < tileCount; ++tz)
		{
			for (int32_t tx = 0; tx < tileCount; ++tx)
				addTileDependencies(pipelineDepends, sourceInstance, heightfieldInstance, heightfieldAsset, outputPath, outputGuid, tx, tz, 0);
		}

		addTileDependencies(pipelineDepends, sourceInstance, heightfieldInstance, heightfieldAsset, outputPath, outputGuid, -1, -1, hf::TileStorage::calculateOverviewSize(size, heightfieldAsset->getWorldExtent().x()));
	}

	pipelineDepends->addDependency< render::ShaderGraph >();
	return true;
}

bool TerrainPipeline::buildOutput(
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
	if (auto imageAsset = dynamic_type_cast< const TerrainImageAsset* >(sourceAsset))
	{
		Ref< const db::Instance > imageInstance = pipelineBuilder->getSourceDatabase()->getInstance(imageAsset->m_instance);
		if (!imageInstance)
		{
			log::error << L"Terrain pipeline failed; unable to get terrain instance of image tile." << Endl;
			return false;
		}

		const hf::TileStorage imageTileStorage(imageInstance, imageAsset->m_tileCount);
		Ref< drawing::Image > image = (imageAsset->m_tileX >= 0) ?
			imageTileStorage.readImageTile(imageAsset->m_name, imageAsset->m_tileX, imageAsset->m_tileZ) :
			imageTileStorage.readImageOverview(imageAsset->m_name, imageAsset->m_overviewSize);
		if (!image)
		{
			log::error << L"Terrain pipeline failed; unable to read tile of \"" << imageAsset->m_name << L"\" image." << Endl;
			return false;
		}

		// Mipmapped tile maps are power-of-2 with first and last texel centered on first and seam pixel.
		if (imageAsset->m_tileX >= 0)
			image = resampleEdgeAligned(image, image->getWidth() - 1, image->getHeight() - 1);

		// Colour map's alpha is unused; compression would weight colour by its zero alpha and tint it.
		Ref< render::TextureOutput > output = createImageOutput();
		output->m_ignoreAlpha = (imageAsset->m_name == L"Color");
		return pipelineBuilder->buildAdHocOutput(output, outputPath, outputGuid, image);
	}

	const TerrainAsset* terrainAsset = checked_type_cast< const TerrainAsset*, false >(sourceAsset);

	// Get heightfield asset and instance.
	Ref< const db::Instance > heightfieldAssetInstance = pipelineBuilder->getSourceDatabase()->getInstance(terrainAsset->getHeightfield());
	if (!heightfieldAssetInstance)
	{
		log::error << L"Heightfield texture pipeline failed; unable to get heightfield asset instance" << Endl;
		return false;
	}

	Ref< const hf::HeightfieldAsset > heightfieldAsset = heightfieldAssetInstance->getObject< const hf::HeightfieldAsset >();
	if (!heightfieldAsset)
	{
		log::error << L"Heightfield texture pipeline failed; unable to get heightfield asset" << Endl;
		return false;
	}

	const hf::TileStorage tileStorage(heightfieldAssetInstance, heightfieldAsset->getTileCount());
	const hf::TileStorage sourceTileStorage(sourceInstance, heightfieldAsset->getTileCount());

	const int32_t size = tileStorage.readSize();
	if (size <= 0)
	{
		log::error << L"Terrain pipeline failed; unable to read size of heightfield." << Endl;
		return false;
	}

	// Generate uids.
	const Guid splatMapGuid = combineGuids(c_guidSplatMapSeed, outputGuid);
	const Guid terrainShaderGuid = combineGuids(c_guidTerrainShaderSeed, outputGuid);
	const Guid surfaceShaderGuid = combineGuids(c_guidSurfaceShaderSeed, outputGuid);

	// Images are built as tiles; tiles without splat image use a simple default.
	const bool haveSplat = sourceTileStorage.haveData(L"Splat");
	const bool haveColor = sourceTileStorage.haveData(L"Color");
	if (!haveSplat)
	{
		Ref< drawing::Image > splatImage = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), 1, 1);
		splatImage->setPixel(0, 0, Color4f(1.0f, 0.0f, 0.0f, 0.0f));

		Ref< render::TextureOutput > splatTexture = new render::TextureOutput();
		splatTexture->m_keepZeroAlpha = false;
		splatTexture->m_ignoreAlpha = false;
		splatTexture->m_hasAlpha = true;
		splatTexture->m_assumeLinearGamma = true;
		splatTexture->m_enableCompression = false;
		pipelineBuilder->buildAdHocOutput(splatTexture, outputPath + L"/Splat", splatMapGuid, splatImage);
	}

	// Read surface shader and prepare with proper input and output ports.
	Ref< const render::ShaderGraph > assetSurfaceShader = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(terrainAsset->getSurfaceShader());
	if (!assetSurfaceShader)
	{
		log::error << L"Terrain pipeline failed; unable to get terrain template shader." << Endl;
		return false;
	}

	Ref< render::ShaderGraph > surfaceShaderImpl = DeepClone(assetSurfaceShader).create< render::ShaderGraph >();

	// Read shader templates.
	Ref< const render::ShaderGraph > terrainShaderTemplate = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(c_guidTerrainShaderTemplate);
	if (!terrainShaderTemplate)
	{
		log::error << L"Terrain pipeline failed; unable to get terrain template shader-" << Endl;
		return false;
	}

	Ref< const render::ShaderGraph > surfaceShaderTemplate = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(c_guidSurfaceShaderTemplate);
	if (!surfaceShaderTemplate)
	{
		log::error << L"Terrain pipeline failed; unable to get surface template shader." << Endl;
		return false;
	}

	// Resolve fragments in templates and insert surface shader at placeholders.
	FragmentReaderAdapter fragmentReader(pipelineBuilder, surfaceShaderImpl);

	Ref< render::ShaderGraph > terrainShader = render::FragmentLinker(fragmentReader).resolve(terrainShaderTemplate, false);
	if (!terrainShader)
	{
		log::error << L"Terrain pipeline failed; unable to link terrain shader." << Endl;
		return false;
	}

	Ref< render::ShaderGraph > surfaceShader = render::FragmentLinker(fragmentReader).resolve(surfaceShaderTemplate, false);
	if (!surfaceShader)
	{
		log::error << L"Terrain pipeline failed; unable to link surface shader." << Endl;
		return false;
	}

	// Build shaders.
	const std::wstring shaderPath = Path(outputPath).getPathOnly() + L"/" + outputGuid.format();

	if (!pipelineBuilder->buildAdHocOutput(
		terrainShader,
		shaderPath,
		terrainShaderGuid
	))
	{
		log::error << L"Terrain pipeline failed; unable to build terrain shader." << Endl;
		return false;
	}

	if (!pipelineBuilder->buildAdHocOutput(
		surfaceShader,
		shaderPath + L"/Surface",
		surfaceShaderGuid
	))
	{
		log::error << L"Terrain pipeline failed; unable to build surface shader." << Endl;
		return false;
	}

	// Create output resource.
	Ref< TerrainResource > terrainResource = new TerrainResource();
	terrainResource->m_detailSkip = terrainAsset->getDetailSkip();
	terrainResource->m_patchDim = terrainAsset->getPatchDim();
	terrainResource->m_terrainShader = resource::Id< render::Shader >(terrainShaderGuid);
	terrainResource->m_surfaceShader = resource::Id< render::Shader >(surfaceShaderGuid);

	// Tile resources are built as dependencies; only their ids are needed here.
	const int32_t tileCount = std::max(heightfieldAsset->getTileCount(), 1);
	terrainResource->m_tileCount = tileCount;
	terrainResource->m_tileSize = size / tileCount;
	for (int32_t tz = 0; tz < tileCount; ++tz)
	{
		for (int32_t tx = 0; tx < tileCount; ++tx)
			terrainResource->m_tiles.push_back(createTile(outputGuid, tx, tz, haveSplat, haveColor, splatMapGuid));
	}
	terrainResource->m_overview = createTile(outputGuid, -1, -1, haveSplat, haveColor, splatMapGuid);

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Terrain pipeline failed; unable to create output instance." << Endl;
		return false;
	}

	outputInstance->setObject(terrainResource);

	if (!outputInstance->commit())
	{
		log::error << L"Terrain pipeline failed; unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

Guid TerrainPipeline::getTileHeightfieldId(const Guid& terrainId, int32_t tileX, int32_t tileZ)
{
	return tileGuid(c_guidTileHeightfieldSeed, terrainId, tileX, tileZ);
}

}
