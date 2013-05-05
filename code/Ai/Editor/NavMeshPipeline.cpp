#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include "Ai/NavMeshResource.h"
#include "Ai/Editor/NavMeshAsset.h"
#include "Ai/Editor/NavMeshPipeline.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/TString.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/Triangulate.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor
{
	namespace ai
	{
		namespace
		{

class BuildContext : public rcContext
{
protected:
	virtual void doLog(const rcLogCategory /*category*/, const char* msg, const int /*len*/)
	{
		log::info << mbstows(msg) << Endl;
	}
};

void copyUnaligned3(float out[3], const Vector4& source)
{
	out[0] = source.x();
	out[1] = source.y();
	out[2] = source.z();
}

template < typename PipelineType >
Ref< ISerializable > resolveAllExternal(PipelineType* pipeline, const ISerializable* object)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (const world::ExternalEntityData* externalEntityDataRef = dynamic_type_cast< const world::ExternalEntityData* >(objectMember->get()))
		{
			Ref< const ISerializable > externalEntityData = pipeline->getObjectReadOnly(externalEntityDataRef->getEntityData());
			if (!externalEntityData)
				return 0;

			Ref< world::EntityData > resolvedEntityData = dynamic_type_cast< world::EntityData* >(resolveAllExternal(pipeline, externalEntityData));
			if (!resolvedEntityData)
				return 0;

			resolvedEntityData->setName(externalEntityDataRef->getName());
			resolvedEntityData->setTransform(externalEntityDataRef->getTransform());

			objectMember->set(resolvedEntityData);
		}
		else if (objectMember->get())
		{
			objectMember->set(resolveAllExternal(pipeline, objectMember->get()));
		}
	}

	return reflection->clone();
}

void collectNavigationEntities(const ISerializable* object, const Transform& transform, RefArray< world::EntityData >& outEntityData)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (mesh::MeshEntityData* meshEntityData = dynamic_type_cast< mesh::MeshEntityData* >(objectMember->get()))
		{
			outEntityData.push_back(meshEntityData);
		}
		else if (terrain::TerrainEntityData* terrainEntityData = dynamic_type_cast< terrain::TerrainEntityData* >(objectMember->get()))
		{
			outEntityData.push_back(terrainEntityData);
		}
		else if (world::LayerEntityData* layerEntityData = dynamic_type_cast< world::LayerEntityData* >(objectMember->get()))
		{
			if (layerEntityData->isDynamic())
				continue;

			collectNavigationEntities(
				objectMember->get(),
				layerEntityData->getTransform(),
				outEntityData
			);
		}
		else if (world::EntityData* entityData = dynamic_type_cast< world::EntityData* >(objectMember->get()))
		{
			collectNavigationEntities(
				objectMember->get(),
				entityData->getTransform(),
				outEntityData
			);
		}
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshPipeline", 5, NavMeshPipeline, editor::DefaultPipeline)

bool NavMeshPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

TypeInfoSet NavMeshPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< NavMeshAsset >());
	return typeSet;
}

bool NavMeshPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const NavMeshAsset* asset = checked_type_cast< const NavMeshAsset*, false >(sourceAsset);
	pipelineDepends->addDependency(asset->m_source, editor::PdfUse);
	return true;
}

bool NavMeshPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
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
	const NavMeshAsset* asset = checked_type_cast< const NavMeshAsset*, false >(sourceAsset);

	Ref< const ISerializable > sourceData = pipelineBuilder->getObjectReadOnly(asset->m_source);
	if (!sourceData)
		return false;

	sourceData = resolveAllExternal(pipelineBuilder, sourceData);

	RefArray< world::EntityData > entityData;
	collectNavigationEntities(sourceData, Transform::identity(), entityData);

	log::info << L"Found " << int32_t(entityData.size()) << L" entity(s)" << Endl;

	RefArray< model::Model > navModels;
	//Ref< model::Model > debugModel = new model::Model();
	Aabb3 navModelsAabb;
	uint32_t navModelsTriangleCount = 0;

	// Load all mesh models, translate and triangulate em.
	{
		std::map< std::wstring, Ref< const model::Model > > modelCache;
		for (RefArray< world::EntityData >::const_iterator i = entityData.begin(); i != entityData.end(); ++i)
		{
			if (const mesh::MeshEntityData* meshEntityData = dynamic_type_cast< const mesh::MeshEntityData* >(*i))
			{
				const resource::Id< mesh::IMesh >& mesh = meshEntityData->getMesh();

				Ref< const mesh::MeshAsset > meshAsset = pipelineBuilder->getObjectReadOnly< mesh::MeshAsset >(mesh);
				if (!meshAsset)
					continue;

				std::map< std::wstring, Ref< const model::Model > >::const_iterator j = modelCache.find(meshAsset->getFileName().getOriginal());
				if (j != modelCache.end())
				{
					Ref< model::Model > navModel = new model::Model(*j->second);

					AlignedVector< Vector4 > positions = navModel->getPositions();
					for (AlignedVector< Vector4 >::iterator k = positions.begin(); k != positions.end(); ++k)
						*k = (*i)->getTransform() * k->xyz1();
					navModel->setPositions(positions);

					navModels.push_back(navModel);
					navModelsAabb.contain(navModel->getBoundingBox());
					navModelsTriangleCount += navModel->getPolygonCount();

					//model::MergeModel(*navModel, Transform::identity()).apply(*debugModel);
				}
				else
				{
					Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), meshAsset->getFileName().getOriginal());
					if (!file)
					{
						log::warning << L"Unable to open file \"" << meshAsset->getFileName().getOriginal() << L"\"" << Endl;
						continue;
					}

					Ref< model::Model > meshModel = model::ModelFormat::readAny(
						file,
						meshAsset->getFileName().getExtension(),
						model::ModelFormat::IfMeshPositions |
						model::ModelFormat::IfMeshVertices |
						model::ModelFormat::IfMeshPolygons
					);
					if (!meshModel)
					{
						log::warning << L"Unable to read model \"" << meshAsset->getFileName().getOriginal() << L"\"" << Endl;
						continue;
					}

					model::Triangulate().apply(*meshModel);

					modelCache[meshAsset->getFileName().getOriginal()] = meshModel;

					Ref< model::Model > navModel = new model::Model(*meshModel);

					AlignedVector< Vector4 > positions = navModel->getPositions();
					for (AlignedVector< Vector4 >::iterator k = positions.begin(); k != positions.end(); ++k)
						*k = (*i)->getTransform() * k->xyz1();
					navModel->setPositions(positions);

					navModels.push_back(navModel);
					navModelsAabb.contain(navModel->getBoundingBox());
					navModelsTriangleCount += navModel->getPolygonCount();

					//model::MergeModel(*navModel, Transform::identity()).apply(*debugModel);
				}
			}
			else if (const terrain::TerrainEntityData* terrainEntityData = dynamic_type_cast< const terrain::TerrainEntityData* >(*i))
			{
				const resource::Id< terrain::Terrain >& terrain = terrainEntityData->getTerrain();

				Ref< const terrain::TerrainAsset > terrainAsset = pipelineBuilder->getObjectReadOnly< terrain::TerrainAsset >(terrain);
				if (!terrain)
					continue;

				Ref< db::Instance > heightfieldAssetInstance = pipelineBuilder->getSourceDatabase()->getInstance(terrainAsset->getHeightfield());
				if (!heightfieldAssetInstance)
					continue;

				Ref< const hf::HeightfieldAsset > heightfieldAsset = heightfieldAssetInstance->getObject< const hf::HeightfieldAsset >();
				if (!heightfieldAsset)
					continue;

				Ref< IStream > sourceData = heightfieldAssetInstance->readData(L"Data");
				if (!sourceData)
					continue;

				Ref< hf::Heightfield > heightfield = hf::HeightfieldFormat().read(
					sourceData,
					heightfieldAsset->getWorldExtent()
				);
				if (!heightfield)
					continue;

				sourceData->close();
				sourceData = 0;

				int32_t step = 16;
				int32_t size = heightfield->getSize();
				int32_t outputSize = size / step;

				Ref< model::Model > navModel = new model::Model();

				navModel->reservePositions(outputSize * outputSize);

				model::Vertex vertex;
				for (int32_t iz = 0; iz < size; iz += step)
				{
					for (int32_t ix = 0; ix < size; ix += step)
					{
						float wx, wz;
						heightfield->gridToWorld(ix, iz, wx, wz);

						uint32_t positionId = navModel->addPosition(Vector4(
							wx,
							heightfield->getWorldHeight(wx, wz),
							wz,
							1.0f
						));

						vertex.setPosition(positionId);
						navModel->addVertex(vertex);
					}
				}

				model::Polygon polygon;
				for (int32_t iz = 0; iz < outputSize - 1; ++iz)
				{
					int32_t offset = iz * outputSize;
					for (int32_t ix = 0; ix < outputSize - 1; ++ix)
					{
						float wx, wz;
						heightfield->gridToWorld(ix * step, iz * step, wx, wz);

						if (!heightfield->getWorldCut(wx, wz))
							continue;
						if (!heightfield->getWorldCut(wx + step, wz))
							continue;
						if (!heightfield->getWorldCut(wx + step, wz + step))
							continue;
						if (!heightfield->getWorldCut(wx, wz + step))
							continue;

						int32_t indices[] =
						{
							offset + ix,
							offset + ix + 1,
							offset + ix + 1 + outputSize,
							offset + ix + outputSize
						};

						polygon.clearVertices();
						polygon.addVertex(indices[0]);
						polygon.addVertex(indices[1]);
						polygon.addVertex(indices[3]);
						navModel->addPolygon(polygon);

						polygon.clearVertices();
						polygon.addVertex(indices[1]);
						polygon.addVertex(indices[2]);
						polygon.addVertex(indices[3]);
						navModel->addPolygon(polygon);
					}
				}

				navModels.push_back(navModel);
				navModelsAabb.contain(navModel->getBoundingBox());
				navModelsTriangleCount += navModel->getPolygonCount();
			}
		}
	}

	//model::ModelFormat::writeAny(L"data/Temp/NavMesh_source.obj", debugModel);
	//debugModel = 0;

	BuildContext ctx;

	rcConfig cfg;

	std::memset(&cfg, 0, sizeof(cfg));
	cfg.cs = asset->m_cellSize;
	cfg.ch = asset->m_cellHeight;
	cfg.walkableSlopeAngle = asset->m_agentSlope;
	cfg.walkableHeight = int(std::ceilf(asset->m_agentHeight / cfg.ch));
	cfg.walkableClimb = int(std::floorf(asset->m_agentClimb / cfg.ch));
	cfg.walkableRadius = int(std::ceilf(asset->m_agentRadius / cfg.cs));
	cfg.maxEdgeLen = int(asset->m_maxEdgeLength / asset->m_cellSize);
	cfg.maxSimplificationError = asset->m_maxSimplificationError;
	cfg.minRegionArea = int(asset->m_minRegionSize * asset->m_minRegionSize);
	cfg.mergeRegionArea = int(asset->m_mergeRegionSize * asset->m_mergeRegionSize);
	cfg.maxVertsPerPoly = 6;
	cfg.detailSampleDist = (asset->m_detailSampleDistance < 0.9f) ? 0.0f : asset->m_cellSize * asset->m_detailSampleDistance;
	cfg.detailSampleMaxError = asset->m_cellHeight * asset->m_detailSampleMaxError;

	copyUnaligned3(cfg.bmin, navModelsAabb.mn);
	copyUnaligned3(cfg.bmax, navModelsAabb.mx);

	rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);

	rcHeightfield* solid = rcAllocHeightfield();
	if (!solid)
		return false;

	if (!rcCreateHeightfield(&ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch))
		return false;

	// Allocate array that can hold triangle area types.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	AutoArrayPtr< uint8_t > triAreas(new uint8_t [navModelsTriangleCount]);
	if (!triAreas.c_ptr())
		return false;

	std::memset(triAreas.ptr(), 0, navModelsTriangleCount * sizeof(uint8_t));

	uint8_t* triAreaPtr = triAreas.ptr();
	for (RefArray< model::Model >::const_iterator i = navModels.begin(); i != navModels.end(); ++i)
	{
		int32_t vertexCount = (*i)->getVertexCount();
		int32_t triangleCount = (*i)->getPolygonCount();

		AutoArrayPtr< float > vertices(new float [3 * vertexCount]);
		for (int32_t j = 0; j < vertexCount; ++j)
		{
			const Vector4& position = (*i)->getVertexPosition(j);
			copyUnaligned3(&vertices[j * 3], position);
		}

		AutoArrayPtr< int32_t > indices(new int32_t [3 * triangleCount]);
		for (int32_t j = 0; j < triangleCount; ++j)
		{
			const model::Polygon& triangle = (*i)->getPolygon(j);
			T_ASSERT (triangle.getVertexCount() == 3);

			indices[j * 3 + 2] = triangle.getVertex(0);
			indices[j * 3 + 1] = triangle.getVertex(1);
			indices[j * 3 + 0] = triangle.getVertex(2);
		}

		rcMarkWalkableTriangles(&ctx, cfg.walkableSlopeAngle, vertices.c_ptr(), vertexCount, indices.c_ptr(), triangleCount, triAreaPtr);
		rcRasterizeTriangles(&ctx, vertices.c_ptr(), vertexCount, indices.c_ptr(), triAreaPtr, triangleCount, *solid, cfg.walkableClimb);

		triAreaPtr += triangleCount;
	}

	//
	// Step 3. Filter walkables surfaces.
	//

	// Once all geometry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, *solid);
	rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
	rcFilterWalkableLowHeightSpans(&ctx, cfg.walkableHeight, *solid);

	//
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	rcCompactHeightfield* chf = rcAllocCompactHeightfield();
	if (!chf)
		return false;

	if (!rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf))
		return false;

	rcFreeHeightField(solid);
	solid = 0;

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(&ctx, cfg.walkableRadius, *chf))
		return false;

	//// (Optional) Mark areas.
	//const ConvexVolume* vols = m_geom->getConvexVolumes();
	//for (int i  = 0; i < m_geom->getConvexVolumeCount(); ++i)
	//	rcMarkConvexPolyArea(ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *chf);

	const bool c_monotonePartitioning = false;
	if (c_monotonePartitioning)
	{
		// Partition the walkable surface into simple regions without holes.
		// Monotone partitioning does not need distancefield.
		if (!rcBuildRegionsMonotone(&ctx, *chf, 0, cfg.minRegionArea, cfg.mergeRegionArea))
			return false;
	}
	else
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if (!rcBuildDistanceField(&ctx, *chf))
			return false;

		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegions(&ctx, *chf, 0, cfg.minRegionArea, cfg.mergeRegionArea))
			return false;
	}

	//
	// Step 5. Trace and simplify region contours.
	//

	// Create contours.
	rcContourSet* cset = rcAllocContourSet();
	if (!cset)
		return false;

	if (!rcBuildContours(&ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset))
		return false;

	//
	// Step 6. Build polygons mesh from contours.
	//

	// Build polygon navmesh from the contours.
	rcPolyMesh* pmesh = rcAllocPolyMesh();
	if (!pmesh)
		return false;

	if (!rcBuildPolyMesh(&ctx, *cset, cfg.maxVertsPerPoly, *pmesh))
		return false;

	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//

	rcPolyMeshDetail* dmesh = rcAllocPolyMeshDetail();
	if (!dmesh)
		return false;

	if (!rcBuildPolyMeshDetail(&ctx, *pmesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh))
		return false;

	rcFreeCompactHeightfield(chf);
	chf = 0;

	rcFreeContourSet(cset);
	cset = 0;

	//
	// Step 8. Create Detour navigation mesh.
	//
	for (int i = 0; i < pmesh->npolys; ++i)
	{
		if (pmesh->areas[i] == RC_WALKABLE_AREA)
			pmesh->flags[i] = 0xffff;
	}

	dtNavMeshCreateParams params;
	std::memset(&params, 0, sizeof(params));

	params.verts = pmesh->verts;
	params.vertCount = pmesh->nverts;
	params.polys = pmesh->polys;
	params.polyAreas = pmesh->areas;
	params.polyFlags = pmesh->flags;
	params.polyCount = pmesh->npolys;
	params.nvp = pmesh->nvp;
	params.detailMeshes = dmesh->meshes;
	params.detailVerts = dmesh->verts;
	params.detailVertsCount = dmesh->nverts;
	params.detailTris = dmesh->tris;
	params.detailTriCount = dmesh->ntris;
	params.walkableHeight = asset->m_agentHeight;
	params.walkableRadius = asset->m_agentRadius;
	params.walkableClimb = asset->m_agentClimb;
	rcVcopy(params.bmin, pmesh->bmin);
	rcVcopy(params.bmax, pmesh->bmax);
	params.cs = cfg.cs;
	params.ch = cfg.ch;
	params.buildBvTree = false;

	uint8_t* navData = 0;
	int32_t navDataSize = 0;
	if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		return false;

	// Save navigation data in resource.
	Ref< NavMeshResource > outputResource = new NavMeshResource();

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(outputResource);

	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Unable to create data stream" << Endl;
		outputInstance->revert();
		return false;
	}

	Writer(stream) << uint8_t(1);
	Writer(stream) << navDataSize;

	if (stream->write(navData, navDataSize) != navDataSize)
	{
		log::error << L"Unable to write to data stream" << Endl;
		outputInstance->revert();
		return false;
	}

	stream->close();
	stream = 0;

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance" << Endl;
		return false;
	}

	dtFree(navData);
	navData = 0;

	// Save pmesh for debugging.
	{
		Ref< model::Model > pmeshModel = new model::Model();

		for (int32_t i = 0; i < pmesh->nverts; ++i)
		{
			pmeshModel->addPosition(Vector4(
				pmesh->bmin[0] + pmesh->verts[i * 3 + 0] * pmesh->cs,
				pmesh->bmin[1] + pmesh->verts[i * 3 + 1] * pmesh->ch,
				pmesh->bmin[2] + pmesh->verts[i * 3 + 2] * pmesh->cs,
				1.0f
			));
			pmeshModel->addVertex(model::Vertex(i));
		}

		for (int32_t i = 0; i < pmesh->npolys; ++i)
		{
			model::Polygon polygon;

			const uint16_t* p = &pmesh->polys[i * pmesh->nvp * 2];
			for (int32_t j = 0; j < pmesh->nvp; ++j)
			{
				if (p[j] == RC_MESH_NULL_IDX)
					break;

				polygon.addVertex(p[j]);
			}

			polygon.flipWinding();

			pmeshModel->addPolygon(polygon);
		}
		
		model::ModelFormat::writeAny(L"data/Temp/NavMesh_nav.obj", pmeshModel);
	}

	rcFreePolyMeshDetail(dmesh);
	dmesh = 0;

	rcFreePolyMesh(pmesh);
	pmesh = 0;

	return true;
}

	}
}
