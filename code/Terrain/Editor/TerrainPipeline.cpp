/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldTextureAsset.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Terrain/TerrainResource.h"
#include "Terrain/TerrainUtilities.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "Terrain/Editor/TerrainPipeline.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const uint32_t c_cutsCountThreshold = 10;

const Guid c_guidSplatMapSeed(L"{3BDF1640-E844-404b-85F2-277C022B8E23}");
const Guid c_guidColorMapSeed(L"{E2A97254-B596-4665-900B-FB70A2267AF7}");
const Guid c_guidNormalMapSeed(L"{84F74E7F-4D02-40f6-A07A-EE9F5EF3CDB4}");
const Guid c_guidHeightMapSeed(L"{EA932687-BC1E-477f-BF70-A8715991258D}");
const Guid c_guidCutMapSeed(L"{CFB69515-9263-4611-93B1-658D8CA6D861}");
const Guid c_guidTerrainShaderSeed(L"{6643B92A-6676-41b9-9427-3569B2EA481B}");
const Guid c_guidSurfaceShaderSeed(L"{8481FC82-A8E8-49b8-906F-9F8F6365B1F5}");
const Guid c_guidTerrainShaderTemplate(L"{A6C4532A-0540-4D42-93FC-964C7BFDD1FD}");
const Guid c_guidSurfaceShaderTemplate(L"{BAD675B3-9799-7D49-A045-BDA471DD5A3E}");
const Guid c_guidSurfaceShaderPlaceholder(L"{23790224-9E2A-4C43-9C3B-F659BE962E10}");

class FragmentReaderAdapter : public render::FragmentLinker::IFragmentReader
{
public:
	explicit FragmentReaderAdapter(editor::IPipelineBuilder* pipelineBuilder, const render::ShaderGraph* surfaceShaderImpl)
	:	m_pipelineBuilder(pipelineBuilder)
	,	m_surfaceShaderImpl(surfaceShaderImpl)
	{
	}

	virtual Ref< const render::ShaderGraph > read(const Guid& fragmentGuid) const
	{
		if (fragmentGuid == c_guidSurfaceShaderPlaceholder)
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

void calculatePatches(const TerrainAsset* terrainAsset, const hf::Heightfield* heightfield, std::vector< TerrainResource::Patch >& outPatches)
{
	const uint32_t patchDim = terrainAsset->getPatchDim();
	const uint32_t detailSkip = terrainAsset->getDetailSkip();

	const uint32_t heightfieldSize = heightfield->getSize();
	const uint32_t patchCount = heightfieldSize / (patchDim * detailSkip);

	outPatches.resize(patchCount * patchCount);

	for (uint32_t pz = 0; pz < patchCount; ++pz)
	{
		for (uint32_t px = 0; px < patchCount; ++px)
		{
			TerrainResource::Patch& patch = outPatches[px + pz * patchCount];
			calculatePatchMinMaxHeight(heightfield, px, pz, patchDim, detailSkip, patch.height);
			calculatePatchErrorMetrics(heightfield, 4, px, pz, patchDim, detailSkip, patch.error);
		}
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainPipeline", 13, TerrainPipeline, editor::DefaultPipeline)

bool TerrainPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	return editor::DefaultPipeline::create(settings);
}

TypeInfoSet TerrainPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< TerrainAsset >();
}

bool TerrainPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const TerrainAsset* terrainAsset = checked_type_cast< const TerrainAsset*, false >(sourceAsset);

	pipelineDepends->addDependency(terrainAsset->getHeightfield(), editor::PdfUse | editor::PdfBuild | editor::PdfResource);
	pipelineDepends->addDependency(terrainAsset->getSurfaceShader(), editor::PdfUse);

	pipelineDepends->addDependency(c_guidTerrainShaderTemplate, editor::PdfUse);

	pipelineDepends->addDependency(c_guidSurfaceShaderTemplate, editor::PdfUse);
	pipelineDepends->addDependency(c_guidSurfaceShaderPlaceholder, editor::PdfUse);

	// Synthesize ids.
	Guid normalMapGuid = combineGuids(c_guidNormalMapSeed, outputGuid);
	Guid heightMapGuid = combineGuids(c_guidHeightMapSeed, outputGuid);
	Guid cutMapGuid = combineGuids(c_guidCutMapSeed, outputGuid);

	// Create normal map.
	Ref< hf::HeightfieldTextureAsset > normalMapAsset = new hf::HeightfieldTextureAsset();
	normalMapAsset->m_heightfield = terrainAsset->getHeightfield();
	normalMapAsset->m_output = hf::HeightfieldTextureAsset::OtNormals;
	normalMapAsset->m_scale = 1.0f;
	pipelineDepends->addDependency(normalMapAsset, outputPath + L"/Normals", normalMapGuid, editor::PdfBuild | editor::PdfResource);

	// Create height map.
	Ref< hf::HeightfieldTextureAsset > heightMapAsset = new hf::HeightfieldTextureAsset();
	heightMapAsset->m_heightfield = terrainAsset->getHeightfield();
	heightMapAsset->m_output = hf::HeightfieldTextureAsset::OtHeights;
	heightMapAsset->m_scale = 1.0f;
	pipelineDepends->addDependency(heightMapAsset, outputPath + L"/Heights", heightMapGuid, editor::PdfBuild | editor::PdfResource);

	// Create cut map. \fixme Do not build cut map if it's not used.
	Ref< hf::HeightfieldTextureAsset > cutMapAsset = new hf::HeightfieldTextureAsset();
	cutMapAsset->m_heightfield = terrainAsset->getHeightfield();
	cutMapAsset->m_output = hf::HeightfieldTextureAsset::OtCuts;
	cutMapAsset->m_scale = 1.0f;
	pipelineDepends->addDependency(cutMapAsset, outputPath + L"/Cuts", cutMapGuid, editor::PdfBuild | editor::PdfResource);

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

	Ref< IStream > sourceData = heightfieldAssetInstance->readData(L"Data");
	if (!sourceData)
	{
		log::error << L"Heightfield pipeline failed; unable to open heights" << Endl;
		return false;
	}

	Ref< hf::Heightfield > heightfield = hf::HeightfieldFormat().read(
		sourceData,
		heightfieldAsset->getWorldExtent()
	);
	if (!heightfield)
	{
		log::error << L"Heightfield pipeline failed; unable to read heights" << Endl;
		return false;
	}

	safeClose(sourceData);

	// Check if heightfield have cuts.
	uint32_t cutsCount = 0;

	const uint8_t* cuts = heightfield->getCuts();
	if (cuts)
	{
		const int32_t size = heightfield->getSize();
		for (int32_t i = 0; i < size * size / 8; ++i)
		{
			if (cuts[i] != 0xff)
				++cutsCount;
		}
	}

	// Generate uids.
	const Guid splatMapGuid = combineGuids(c_guidSplatMapSeed, outputGuid);
	const Guid normalMapGuid = combineGuids(c_guidNormalMapSeed, outputGuid);
	const Guid heightMapGuid = combineGuids(c_guidHeightMapSeed, outputGuid);
	const Guid cutMapGuid = (cutsCount >= c_cutsCountThreshold) ? combineGuids(c_guidCutMapSeed, outputGuid) : Guid();
	const Guid terrainShaderGuid = combineGuids(c_guidTerrainShaderSeed, outputGuid);
	const Guid surfaceShaderGuid = combineGuids(c_guidSurfaceShaderSeed, outputGuid);
	Guid colorMapGuid;

	// Create color texture.
	Ref< IStream > file = sourceInstance->readData(L"Color");
	if (file)
	{
		Ref< drawing::Image > colorImage = drawing::Image::load(file, L"tri");
		if (!colorImage)
		{
			file->seek(IStream::SeekSet, 0);
			colorImage = drawing::Image::load(file, L"tga");
		}
		if (!colorImage)
		{
			log::error << L"Terrain pipeline failed; unable to read attached color image" << Endl;
			return false;
		}

		file->close();
		file = nullptr;

		log::info << L"Terrain color map size " << colorImage->getWidth() << L" * " << colorImage->getHeight() << Endl;

		colorMapGuid = combineGuids(c_guidColorMapSeed, outputGuid);

		Ref< render::TextureOutput > colorTexture = new render::TextureOutput();
		colorTexture->m_keepZeroAlpha = false;
		colorTexture->m_ignoreAlpha = false;
		colorTexture->m_hasAlpha = true;
		colorTexture->m_assumeLinearGamma = true;
		colorTexture->m_systemTexture = true;
		pipelineBuilder->buildAdHocOutput(colorTexture, outputPath + L"/Colors", colorMapGuid, colorImage);
	}

	// Create splat texture.
	file = sourceInstance->readData(L"Splat");
	if (file)
	{
		Ref< drawing::Image > splatImage = drawing::Image::load(file, L"tri");
		if (!splatImage)
		{
			file->seek(IStream::SeekSet, 0);
			splatImage = drawing::Image::load(file, L"tga");
		}
		if (!splatImage)
		{
			log::error << L"Terrain pipeline failed; unable to read attached splat image" << Endl;
			return false;
		}

		file->close();
		file = nullptr;

		log::info << L"Terrain splat map size " << splatImage->getWidth() << L" * " << splatImage->getHeight() << Endl;

		Ref< render::TextureOutput > splatTexture = new render::TextureOutput();
		splatTexture->m_keepZeroAlpha = false;
		splatTexture->m_ignoreAlpha = false;
		splatTexture->m_hasAlpha = true;
		splatTexture->m_assumeLinearGamma = true;
		splatTexture->m_systemTexture = true;
		pipelineBuilder->buildAdHocOutput(splatTexture, outputPath + L"/Splat", splatMapGuid, splatImage);
	}
	else
	{
		// No splat image in instance; create simple default.
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
	terrainResource->m_heightfield = terrainAsset->getHeightfield();
	terrainResource->m_colorMap = resource::Id< render::ITexture >(colorMapGuid);
	terrainResource->m_normalMap = resource::Id< render::ITexture >(normalMapGuid);
	terrainResource->m_heightMap = resource::Id< render::ITexture >(heightMapGuid);
	terrainResource->m_splatMap = resource::Id< render::ITexture >(splatMapGuid);
	terrainResource->m_terrainShader = resource::Id< render::Shader >(terrainShaderGuid);
	terrainResource->m_surfaceShader = resource::Id< render::Shader >(surfaceShaderGuid);

	if (cutsCount >= c_cutsCountThreshold)
		terrainResource->m_cutMap = resource::Id< render::ITexture >(cutMapGuid);

	// Calculate lod errors for each terrain patch.
	calculatePatches(terrainAsset, heightfield, terrainResource->m_patches);

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

	}
}
