/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineReport.h"
#include "Editor/IPipelineSettings.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/CalculateConvexHull.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/ScaleAlongNormal.h"
#include "Model/Operations/Triangulate.h"
#include "Physics/Mesh.h"
#include "Physics/MeshResource.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/Editor/MeshPipeline.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.MeshPipeline", 10, MeshPipeline, editor::IPipeline)

bool MeshPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

void MeshPipeline::destroy()
{
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
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const MeshAsset* meshAsset = checked_type_cast< const MeshAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), meshAsset->getFileName().getOriginal());
	return true;
}

bool MeshPipeline::buildOutput(
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
	const MeshAsset* meshAsset = checked_type_cast< const MeshAsset* >(sourceAsset);
	Ref< model::Model > model;

	// We allow models to be passed as build parameters in case models
	// are procedurally generated.
	if (buildParams)
	{
		log::info << L"Using parameter model" << Endl;
		model = checked_type_cast< model::Model* >(
			const_cast< Object* >(buildParams)
		);
	}
	else
	{
		Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), meshAsset->getFileName().getOriginal());
		if (!file)
		{
			log::error << L"Phys mesh pipeline failed; unable to open source model (" << meshAsset->getFileName().getOriginal() << L")" << Endl;
			return false;
		}

		model = model::ModelFormat::readAny(file, meshAsset->getFileName().getExtension());
	}

	if (!model)
	{
		log::error << L"Phys mesh pipeline failed; no model" << Endl;
		return false;
	}
	if (model->getPositions().empty() || model->getVertices().empty() || model->getPolygons().empty())
	{
		log::error << L"Phys mesh pipeline failed; no geometry" << Endl;
		return false;
	}

	// Cleanup model suitable for physics.
	model->clear(model::Model::CfMaterials | model::Model::CfColors | model::Model::CfNormals | model::Model::CfTexCoords | model::Model::CfJoints);
	model::CleanDuplicates(0.01f).apply(*model);

	// Shrink model by margin; need to calculate normals from positions only
	// as we don't want smooth groups or anything else mess with the normals.
	if (abs(meshAsset->m_margin) > FUZZY_EPSILON)
	{
		model::CalculateTangents().apply(*model);
		model::ScaleAlongNormal(-meshAsset->m_margin).apply(*model);
		model->clear(model::Model::CfNormals);
		model::CleanDuplicates(0.01f).apply(*model);
	}

	// Triangulate and ensure no degenerate polygons.
	model::Triangulate().apply(*model);
	model::CleanDegenerate().apply(*model);
	model::CleanDuplicates(0.01f).apply(*model);

	// Calculate bounding box; used for center of gravity estimation.
	Aabb3 boundingBox = model->getBoundingBox();
	Vector4 centerOfGravity = boundingBox.getCenter().xyz0();

	// Create physics mesh.
	AlignedVector< Vector4 > positions = model->getPositions();
	for (AlignedVector< Vector4 >::iterator i = positions.begin(); i != positions.end(); ++i)
		*i -= centerOfGravity;

	AlignedVector< Vector4 > normals;
	AlignedVector< Mesh::Triangle > meshShapeTriangles;
	AlignedVector< Mesh::Triangle > meshHullTriangles;
	AlignedVector< uint32_t > meshHullIndices;

	const std::vector< model::Polygon >& shapeTriangles = model->getPolygons();
	for (std::vector< model::Polygon >::const_iterator i = shapeTriangles.begin(); i != shapeTriangles.end(); ++i)
	{
		T_ASSERT (i->getVertices().size() == 3);

		Mesh::Triangle shapeTriangle;
		for (int j = 0; j < 3; ++j)
			shapeTriangle.indices[j] = model->getVertex(i->getVertex(j)).getPosition();

		meshShapeTriangles.push_back(shapeTriangle);

		Vector4 e1 = positions[shapeTriangle.indices[2]] - positions[shapeTriangle.indices[1]];
		Vector4 e2 = positions[shapeTriangle.indices[0]] - positions[shapeTriangle.indices[1]];

		normals.push_back(cross(e2, e1).normalized());
	}

	if (meshAsset->m_calculateConvexHull)
	{
		log::info << L"Calculating convex hull..." << Endl;

		model::Model hull = *model;
		model::CalculateConvexHull().apply(hull);

		// Extract hull triangles.
		const std::vector< model::Polygon >& hullTriangles = hull.getPolygons();
		for (std::vector< model::Polygon >::const_iterator i = hullTriangles.begin(); i != hullTriangles.end(); ++i)
		{
			T_ASSERT (i->getVertices().size() == 3);

			Mesh::Triangle hullTriangle;
			for (int j = 0; j < 3; ++j)
				hullTriangle.indices[j] = hull.getVertex(i->getVertex(j)).getPosition();

			meshHullTriangles.push_back(hullTriangle);
		}

		// Extract hull indices.
		std::set< uint32_t > uniqueIndices;
		for (std::vector< model::Polygon >::const_iterator i = hullTriangles.begin(); i != hullTriangles.end(); ++i)
		{
			for (int j = 0; j < 3; ++j)
				uniqueIndices.insert(hull.getVertex(i->getVertex(j)).getPosition());
		}
		for (std::set< uint32_t >::const_iterator i = uniqueIndices.begin(); i != uniqueIndices.end(); ++i)
			meshHullIndices.push_back(*i);

		// Improve center of gravity by weighting in volumes of each hull face tetrahedron.
		Vector4 Voffset = Vector4::zero();
		float Vtotal = 0.0f;
		for (AlignedVector< Mesh::Triangle >::const_iterator i = meshHullTriangles.begin(); i != meshHullTriangles.end(); ++i)
		{
			const Vector4 a = positions[i->indices[0]];
			const Vector4 b = positions[i->indices[1]];
			const Vector4 c = positions[i->indices[2]];

			float V = abs(dot3(a, cross(b, c))) / 6.0f;
			Vector4 C = (a + b + c) / Scalar(4.0f);

			Voffset += C * Scalar(V);
			Vtotal += V;
		}

		for (AlignedVector< Vector4 >::iterator i = positions.begin(); i != positions.end(); ++i)
			*i -= Voffset / Scalar(Vtotal);

		centerOfGravity += Voffset / Scalar(Vtotal);
		log::info << L"Hull volume " << Vtotal << L" unit^3" << Endl;
	}

	// Log statistics.
	log::info << int32_t(positions.size()) << L" vertex(es)" << Endl;
	log::info << int32_t(meshShapeTriangles.size()) << L" shape triangle(s)" << Endl;
	if (meshAsset->m_calculateConvexHull)
	{
		log::info << int32_t(meshHullTriangles.size()) << L" hull triangle(s)" << Endl;
		log::info << L"Offset " << centerOfGravity << Endl;
	}
	log::info << meshAsset->m_margin << L" unit(s) margin" << Endl;

	Mesh mesh;
	mesh.setVertices(positions);
	mesh.setNormals(normals);
	mesh.setShapeTriangles(meshShapeTriangles);
	mesh.setHullTriangles(meshHullTriangles);
	mesh.setHullIndices(meshHullIndices);
	mesh.setOffset(centerOfGravity);
	mesh.setMargin(meshAsset->m_margin);

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Phys mesh pipeline failed; unable create output instance" << Endl;
		return false;
	}

	instance->setObject(new MeshResource());

	Ref< IStream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Phys mesh pipeline failed; unable to write data" << Endl;
		instance->revert();
		return false;
	}

	int64_t dataSize = stream->tell();

	mesh.write(stream);

	dataSize = stream->tell() - dataSize;
	stream->close();

	// Commit resource.
	if (!instance->commit())
	{
		log::error << L"Phys mesh pipeline failed; unable to commit output instance" << Endl;
		return false;
	}

	// Create report.
	Ref< editor::IPipelineReport > report = pipelineBuilder->createReport(L"PhysMesh", outputGuid);
	if (report)
	{
		report->set(L"path", outputPath);
		report->set(L"vertexCount", positions.size());
		report->set(L"shapeTriangleCount", meshShapeTriangles.size());
		report->set(L"hullTriangleCount", meshHullTriangles.size());
		report->set(L"size", int32_t(dataSize));
	}

	return true;
}

Ref< ISerializable > MeshPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
