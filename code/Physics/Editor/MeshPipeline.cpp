#include "Physics/Editor/MeshPipeline.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/MeshResource.h"
#include "Physics/Mesh.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineSettings.h"
#include "Database/Instance.h"
#include "Model/Formats/ModelFormat.h"
#include "Model/Model.h"
#include "Model/Utilities.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.MeshPipeline", MeshPipeline, editor::IPipeline)

bool MeshPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< editor::PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void MeshPipeline::destroy()
{
}

uint32_t MeshPipeline::getVersion() const
{
	return 1;
}

TypeInfoSet MeshPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MeshAsset >());
	return typeSet;
}

bool MeshPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const MeshAsset* meshAsset = checked_type_cast< const MeshAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, meshAsset->getFileName());
	pipelineDepends->addDependency(fileName);
	return true;
}

bool MeshPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const MeshAsset* meshAsset = checked_type_cast< const MeshAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, meshAsset->getFileName());

	// Import source model.
	Ref< model::Model > model = model::ModelFormat::readAny(fileName);
	if (!model)
		return false;

	model::triangulateModel(*model);

	model::Model hull = *model;
	model::calculateConvexHull(hull);

	// Create physics mesh.
	std::vector< Mesh::Triangle > outShapeTriangles;
	std::vector< Mesh::Triangle > outHullTriangles;

	const std::vector< model::Polygon >& shapeTriangles = model->getPolygons();
	const std::vector< model::Polygon >& hullTriangles = hull.getPolygons();

	for (std::vector< model::Polygon >::const_iterator i = shapeTriangles.begin(); i != shapeTriangles.end(); ++i)
	{
		T_ASSERT (i->getVertices().size() == 3);

		Mesh::Triangle shapeTriangle;
		for (int j = 0; j < 3; ++j)
			shapeTriangle.indices[j] = model->getVertex(i->getVertex(j)).getPosition();

		outShapeTriangles.push_back(shapeTriangle);
	}

	for (std::vector< model::Polygon >::const_iterator i = hullTriangles.begin(); i != hullTriangles.end(); ++i)
	{
		T_ASSERT (i->getVertices().size() == 3);

		Mesh::Triangle hullTriangle;
		for (int j = 0; j < 3; ++j)
			hullTriangle.indices[j] = hull.getVertex(i->getVertex(j)).getPosition();

		outHullTriangles.push_back(hullTriangle);
	}

	Mesh mesh;

	mesh.setVertices(model->getPositions());
	mesh.setShapeTriangles(outShapeTriangles);
	mesh.setHullTriangles(outHullTriangles);

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid		
	);
	if (!instance)
		return false;

	instance->setObject(new MeshResource());

	Ref< IStream > stream = instance->writeData(L"Data");
	mesh.write(stream);
	stream->close();

	return instance->commit();
}

	}
}
