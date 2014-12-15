#include <limits>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
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
#include "Render/Resource/FragmentLinker.h"
#include "Render/Shader/External.h"
#include "Render/Shader/ShaderGraph.h"
#include "Terrain/TerrainResource.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "Terrain/Editor/TerrainPipeline.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const float c_terrainNormalScale = 0.8f;
const uint32_t c_cutsCountThreshold = 10;

const Guid c_guidSplatMapSeed(L"{3BDF1640-E844-404b-85F2-277C022B8E23}");
const Guid c_guidColorMapSeed(L"{E2A97254-B596-4665-900B-FB70A2267AF7}");
const Guid c_guidNormalMapSeed(L"{84F74E7F-4D02-40f6-A07A-EE9F5EF3CDB4}");
const Guid c_guidHeightMapSeed(L"{EA932687-BC1E-477f-BF70-A8715991258D}");
const Guid c_guidCutMapSeed(L"{CFB69515-9263-4611-93B1-658D8CA6D861}");
const Guid c_guidTerrainCoarseShaderSeed(L"{6643B92A-6676-41b9-9427-3569B2EA481B}");
const Guid c_guidTerrainDetailShaderSeed(L"{1AC67694-4CF8-44ac-B78E-B1E79C9632C8}");
const Guid c_guidSurfaceShaderSeed(L"{8481FC82-A8E8-49b8-906F-9F8F6365B1F5}");
const Guid c_guidTerrainCoarseShaderTemplate(L"{E18056AF-BC95-4349-A98F-17DCF37607D3}");
const Guid c_guidTerrainDetailShaderTemplate(L"{F08984BF-AC87-9A4E-B739-B6F574393F8F}");
const Guid c_guidTerrainCoarseShaderTemplate_VFetch(L"{A6C4532A-0540-4D42-93FC-964C7BFDD1FD}");
const Guid c_guidTerrainDetailShaderTemplate_VFetch(L"{68565BF3-8F72-8848-8FBA-395B9699F108}");
const Guid c_guidSurfaceShaderTemplate(L"{BAD675B3-9799-7D49-A045-BDA471DD5A3E}");
const Guid c_guidSurfaceShaderPlaceholder(L"{23790224-9E2A-4C43-9C3B-F659BE962E10}");

class FragmentReaderAdapter : public render::FragmentLinker::IFragmentReader
{
public:
	FragmentReaderAdapter(editor::IPipelineBuilder* pipelineBuilder, const render::ShaderGraph* surfaceShaderImpl)
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
	uint32_t patchDim = terrainAsset->getPatchDim();
	uint32_t detailSkip = terrainAsset->getDetailSkip();

	uint32_t heightfieldSize = heightfield->getSize();
	uint32_t patchCount = heightfieldSize / (patchDim * detailSkip);

	const Vector4& worldExtent = heightfield->getWorldExtent();

	outPatches.resize(patchCount * patchCount);

	for (uint32_t pz = 0; pz < patchCount; ++pz)
	{
		for (uint32_t px = 0; px < patchCount; ++px)
		{
			TerrainResource::Patch& patch = outPatches[px + pz * patchCount];

			int32_t pminX = (heightfieldSize * px) / patchCount;
			int32_t pminZ = (heightfieldSize * pz) / patchCount;
			int32_t pmaxX = (heightfieldSize * (px + 1)) / patchCount;
			int32_t pmaxZ = (heightfieldSize * (pz + 1)) / patchCount;

			// Measure min and max height of patch.
			float minHeight =  std::numeric_limits< float >::max();
			float maxHeight = -std::numeric_limits< float >::max();

			for (int32_t z = pminZ; z <= pmaxZ; ++z)
			{
				for (int32_t x = pminX; x <= pmaxX; ++x)
				{
					float height = heightfield->getGridHeightNearest(x, z);
					height = heightfield->unitToWorld(height);

					minHeight = min(minHeight, height);
					maxHeight = max(maxHeight, height);
				}
			}

			patch.height[0] = minHeight;
			patch.height[1] = maxHeight;

			// Calculate lod errors.
			float* error = patch.error;
			for (uint32_t lod = 1; lod < /*LodCount*/4; ++lod)
			{
				*error = 0.0f;

				uint32_t lodSkip = 1 << lod;
				for (uint32_t z = 0; z < patchDim; z += lodSkip)
				{
					for (uint32_t x = 0; x < patchDim; x += lodSkip)
					{
						float fx0 = float(x) / (patchDim - 1);
						float fz0 = float(z) / (patchDim - 1);
						float fx1 = float(x + lodSkip) / (patchDim - 1);
						float fz1 = float(z + lodSkip) / (patchDim - 1);

						float gx0 = (fx0 * patchDim * detailSkip) + pminX;
						float gz0 = (fz0 * patchDim * detailSkip) + pminZ;
						float gx1 = (fx1 * patchDim * detailSkip) + pminX;
						float gz1 = (fz1 * patchDim * detailSkip) + pminZ;

						float h[] =
						{
							heightfield->getGridHeightBilinear(gx0, gz0),
							heightfield->getGridHeightBilinear(gx1, gz0),
							heightfield->getGridHeightBilinear(gx0, gz1),
							heightfield->getGridHeightBilinear(gx1, gz1)
						};

						for (uint32_t lz = 0; lz <= lodSkip; ++lz)
						{
							for (uint32_t lx = 0; lx <= lodSkip; ++lx)
							{
								float fx = float(lx) / lodSkip;
								float fz = float(lz) / lodSkip;

								float gx = lerp(gx0, gx1, fx);
								float gz = lerp(gz0, gz1, fz);

								float ht = lerp(h[0], h[1], fx);
								float hb = lerp(h[2], h[3], fx);
								float h0 = lerp(ht, hb, fz);

								float hl = lerp(h[0], h[2], fz);
								float hr = lerp(h[1], h[3], fz);
								float h1 = lerp(hl, hr, fx);

								float h = heightfield->getGridHeightBilinear(gx, gz);

								float herr0 = abs(h - h0);
								float herr1 = abs(h - h1);
								float herr = max(herr0, herr1);

								herr = heightfield->getWorldExtent().y() * herr;

								*error += herr;
							}
						}
					}
				}

				*error++ /= 1000.0f;
			}
		}
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainPipeline", 11, TerrainPipeline, editor::DefaultPipeline)

TerrainPipeline::TerrainPipeline()
:	m_suppressDetailShader(false)
{
}

bool TerrainPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	m_suppressDetailShader = settings->getProperty< PropertyBoolean >(L"TerrainPipeline.SuppressDetailShader", false);
	return editor::DefaultPipeline::create(settings);
}

TypeInfoSet TerrainPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TerrainAsset >());
	return typeSet;
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
	pipelineDepends->addDependency(terrainAsset->getMaterialMask(), editor::PdfUse | editor::PdfBuild | editor::PdfResource);
	pipelineDepends->addDependency(terrainAsset->getSurfaceShader(), editor::PdfUse);

	pipelineDepends->addDependency(c_guidTerrainCoarseShaderTemplate, editor::PdfUse);
	pipelineDepends->addDependency(c_guidTerrainCoarseShaderTemplate_VFetch, editor::PdfUse);
	
	if (!m_suppressDetailShader)
	{
		pipelineDepends->addDependency(c_guidTerrainDetailShaderTemplate, editor::PdfUse);
		pipelineDepends->addDependency(c_guidTerrainDetailShaderTemplate_VFetch, editor::PdfUse);
	}

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
		return 0;
	}

	sourceData->close();
	sourceData = 0;

	// Check if heightfield have cuts.
	uint32_t cutsCount = 0;

	const uint8_t* cuts = heightfield->getCuts();
	if (cuts)
	{
		int32_t size = heightfield->getSize();
		for (int32_t i = 0; i < size * size / 8; ++i)
		{
			if (cuts[i] != 0xff)
				++cutsCount;
		}
	}

	// Generate uids.
	Guid colorMapGuid;
	Guid splatMapGuid = combineGuids(c_guidSplatMapSeed, outputGuid);
	Guid normalMapGuid = combineGuids(c_guidNormalMapSeed, outputGuid);
	Guid heightMapGuid = combineGuids(c_guidHeightMapSeed, outputGuid);
	Guid cutMapGuid = (cutsCount >= c_cutsCountThreshold) ? combineGuids(c_guidCutMapSeed, outputGuid) : Guid();
	Guid terrainCoarseShaderGuid = combineGuids(c_guidTerrainCoarseShaderSeed, outputGuid);
	Guid terrainDetailShaderGuid = combineGuids(c_guidTerrainDetailShaderSeed, outputGuid);
	Guid surfaceShaderGuid = combineGuids(c_guidSurfaceShaderSeed, outputGuid);

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
		file = 0;

		log::info << L"Terrain color map size " << colorImage->getWidth() << L" * " << colorImage->getHeight() << Endl;

		colorMapGuid = combineGuids(c_guidColorMapSeed, outputGuid);

		Ref< render::TextureOutput > colorTexture = new render::TextureOutput();
		colorTexture->m_keepZeroAlpha = false;
		colorTexture->m_ignoreAlpha = false;
		colorTexture->m_hasAlpha = true;
		colorTexture->m_linearGamma = true;
		colorTexture->m_systemTexture = true;
		pipelineBuilder->buildOutput(colorTexture, outputPath + L"/Colors", colorMapGuid, colorImage);
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
		file = 0;

		log::info << L"Terrain splat map size " << splatImage->getWidth() << L" * " << splatImage->getHeight() << Endl;

		Ref< render::TextureOutput > splatTexture = new render::TextureOutput();
		splatTexture->m_keepZeroAlpha = false;
		splatTexture->m_ignoreAlpha = false;
		splatTexture->m_hasAlpha = true;
		splatTexture->m_linearGamma = true;
		splatTexture->m_systemTexture = true;
		pipelineBuilder->buildOutput(splatTexture, outputPath + L"/Splat", splatMapGuid, splatImage);
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
		splatTexture->m_linearGamma = true;
		splatTexture->m_enableCompression = false;
		pipelineBuilder->buildOutput(splatTexture, outputPath + L"/Splat", splatMapGuid, splatImage);
	}

	// Read surface shader and prepare with proper input and output ports.
	Ref< const render::ShaderGraph > assetSurfaceShader = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(terrainAsset->getSurfaceShader());
	if (!assetSurfaceShader)
	{
		log::error << L"Terrain pipeline failed; unable to get terrain template shader" << Endl;
		return false;
	}

	Ref< render::ShaderGraph > surfaceShaderImpl = DeepClone(assetSurfaceShader).create< render::ShaderGraph >();

	// Read shader templates.
	Ref< const render::ShaderGraph > terrainCoarseShaderTemplate = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(c_guidTerrainCoarseShaderTemplate/*_VFetch*/);
	if (!terrainCoarseShaderTemplate)
	{
		log::error << L"Terrain pipeline failed; unable to get terrain coarse template shader" << Endl;
		return false;
	}

	Ref< const render::ShaderGraph > terrainDetailShaderTemplate = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(c_guidTerrainDetailShaderTemplate/*_VFetch*/);
	if (!terrainDetailShaderTemplate)
	{
		log::error << L"Terrain pipeline failed; unable to get terrain detail template shader" << Endl;
		return false;
	}

	Ref< const render::ShaderGraph > surfaceShaderTemplate = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(c_guidSurfaceShaderTemplate);
	if (!surfaceShaderTemplate)
	{
		log::error << L"Terrain pipeline failed; unable to get surface template shader" << Endl;
		return false;
	}

	// Resolve fragments in templates and insert surface shader at placeholders.
	FragmentReaderAdapter fragmentReader(pipelineBuilder, surfaceShaderImpl);

	Ref< render::ShaderGraph > terrainCoarseShader = render::FragmentLinker(fragmentReader).resolve(terrainCoarseShaderTemplate, true);
	if (!terrainCoarseShader)
	{
		log::error << L"Terrain pipeline failed; unable to link terrain coarse shader" << Endl;
		return false;
	}

	Ref< render::ShaderGraph > terrainDetailShader;
	if (!m_suppressDetailShader)
	{
		terrainDetailShader = render::FragmentLinker(fragmentReader).resolve(terrainDetailShaderTemplate, true);
		if (!terrainDetailShader)
		{
			log::error << L"Terrain pipeline failed; unable to link terrain detail shader" << Endl;
			return false;
		}
	}
	else
	{
		terrainDetailShader = DeepClone(terrainCoarseShader).create< render::ShaderGraph >();
		T_ASSERT (terrainDetailShader);
	}

	Ref< render::ShaderGraph > surfaceShader = render::FragmentLinker(fragmentReader).resolve(surfaceShaderTemplate, true);
	if (!surfaceShader)
	{
		log::error << L"Terrain pipeline failed; unable to link surface shader" << Endl;
		return false;
	}

	// Build shaders.
	std::wstring shaderPath = Path(outputPath).getPathOnly() + L"/" + outputGuid.format();

	if (!pipelineBuilder->buildOutput(
		terrainCoarseShader,
		shaderPath + L"/Coarse",
		terrainCoarseShaderGuid
	))
	{
		log::error << L"Terrain pipeline failed; unable to build coarse shader" << Endl;
		return false;
	}

	if (!pipelineBuilder->buildOutput(
		terrainDetailShader,
		shaderPath + L"/Detail",
		terrainDetailShaderGuid
	))
	{
		log::error << L"Terrain pipeline failed; unable to build detail shader" << Endl;
		return false;
	}

	if (!pipelineBuilder->buildOutput(
		surfaceShader,
		shaderPath + L"/Surface",
		surfaceShaderGuid
	))
	{
		log::error << L"Terrain pipeline failed; unable to build surface shader" << Endl;
		return false;
	}

	// Create output resource.
	Ref< TerrainResource > terrainResource = new TerrainResource();
	terrainResource->m_detailSkip = terrainAsset->getDetailSkip();
	terrainResource->m_patchDim = terrainAsset->getPatchDim();
	terrainResource->m_heightfield = terrainAsset->getHeightfield();
	terrainResource->m_materialMask = terrainAsset->getMaterialMask();
	terrainResource->m_colorMap = resource::Id< render::ISimpleTexture >(colorMapGuid);
	terrainResource->m_normalMap = resource::Id< render::ISimpleTexture >(normalMapGuid);
	terrainResource->m_heightMap = resource::Id< render::ISimpleTexture >(heightMapGuid);
	terrainResource->m_splatMap = resource::Id< render::ISimpleTexture >(splatMapGuid);
	terrainResource->m_terrainCoarseShader = resource::Id< render::Shader >(terrainCoarseShaderGuid);
	terrainResource->m_terrainDetailShader = resource::Id< render::Shader >(terrainDetailShaderGuid);
	terrainResource->m_surfaceShader = resource::Id< render::Shader >(surfaceShaderGuid);

	if (cutsCount >= c_cutsCountThreshold)
		terrainResource->m_cutMap = resource::Id< render::ISimpleTexture >(cutMapGuid);

	// Calculate lod errors for each terrain patch.
	calculatePatches(terrainAsset, heightfield, terrainResource->m_patches);

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(terrainResource);

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

	}
}
