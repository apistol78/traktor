#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Heightfield/Editor/HeightfieldTextureAsset.h"
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

const Guid c_guidNormalMapSeed(L"{84F74E7F-4D02-40f6-A07A-EE9F5EF3CDB4}");
const Guid c_guidHeightMapSeed(L"{EA932687-BC1E-477f-BF70-A8715991258D}");

Guid combineGuids(const Guid& g1, const Guid& g2)
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = g1[i] ^ g2[i];
	return Guid(d);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainPipeline", 0, TerrainPipeline, editor::DefaultPipeline)

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
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	const TerrainAsset* terrainAsset = checked_type_cast< const TerrainAsset*, false >(sourceAsset);

	pipelineDepends->addDependency(terrainAsset->getHeightfield(), editor::PdfUse | editor::PdfBuild);

	const std::vector< resource::Id< render::Shader > >& surfaceLayers = terrainAsset->getSurfaceLayers();
	for (std::vector< resource::Id< render::Shader > >::const_iterator i = surfaceLayers.begin(); i != surfaceLayers.end(); ++i)
		pipelineDepends->addDependency(*i, editor::PdfBuild);

	// Synthesize ids.
	Guid normalMapGuid = combineGuids(c_guidNormalMapSeed, outputGuid);
	Guid heightMapGuid = combineGuids(c_guidHeightMapSeed, outputGuid);

	// Create normal map.
	Ref< hf::HeightfieldTextureAsset > normalMapAsset = new hf::HeightfieldTextureAsset();
	normalMapAsset->m_heightfield = terrainAsset->getHeightfield();
	normalMapAsset->m_output = hf::HeightfieldTextureAsset::OtNormals;
	normalMapAsset->m_scale = 1.0f;
	pipelineDepends->addDependency(normalMapAsset, outputPath + L"/Normals", normalMapGuid, editor::PdfBuild);

	// Create height map.
	Ref< hf::HeightfieldTextureAsset > heightMapAsset = new hf::HeightfieldTextureAsset();
	heightMapAsset->m_heightfield = terrainAsset->getHeightfield();
	heightMapAsset->m_output = hf::HeightfieldTextureAsset::OtHeights;
	heightMapAsset->m_scale = 1.0f;
	pipelineDepends->addDependency(heightMapAsset, outputPath + L"/Heights", heightMapGuid, editor::PdfBuild);

	return true;
}

bool TerrainPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const TerrainAsset* terrainAsset = checked_type_cast< const TerrainAsset*, false >(sourceAsset);

	// Synthesize ids.
	Guid normalMapGuid = combineGuids(c_guidNormalMapSeed, outputGuid);
	Guid heightMapGuid = combineGuids(c_guidHeightMapSeed, outputGuid);

	// Create output resource.
	Ref< TerrainResource > terrainResource = new TerrainResource();
	terrainResource->m_heightfield = terrainAsset->getHeightfield();
	terrainResource->m_normalMap = resource::Id< render::ISimpleTexture >(normalMapGuid);
	terrainResource->m_heightMap = resource::Id< render::ISimpleTexture >(heightMapGuid);
	terrainResource->m_surfaceLayers = terrainAsset->getSurfaceLayers();

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
