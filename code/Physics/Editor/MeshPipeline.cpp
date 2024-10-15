/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/Operations/CalculateConvexHull.h"
#include "Model/Operations/CalculateNormals.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/ScaleAlongNormal.h"
#include "Model/Operations/Transform.h"
#include "Model/Operations/Triangulate.h"
#include "Physics/Mesh.h"
#include "Physics/MeshResource.h"
#include "Physics/Editor/Material.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/Editor/MeshPipeline.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.MeshPipeline", 12, MeshPipeline, editor::IPipeline)

bool MeshPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	m_modelCachePath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.ModelCache.Path");
	return true;
}

void MeshPipeline::destroy()
{
}

TypeInfoSet MeshPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< MeshAsset >();
}

bool MeshPipeline::shouldCache() const
{
	return true;
}

uint32_t MeshPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool MeshPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const MeshAsset* meshAsset = mandatory_non_null_type_cast< const MeshAsset* >(sourceAsset);

	if (!meshAsset->getFileName().empty())
		pipelineDepends->addDependency(Path(m_assetPath), meshAsset->getFileName().getOriginal());

	for (auto it : meshAsset->getMaterials())
		pipelineDepends->addDependency(it.second, editor::PdfUse);

	return true;
}

bool MeshPipeline::buildOutput(
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
	const MeshAsset* meshAsset = mandatory_non_null_type_cast< const MeshAsset* >(sourceAsset);
	Ref< model::Model > model;

	// We allow models to be passed as build parameters in case models
	// are procedurally generated.
	if (buildParams)
	{
		model = mandatory_non_null_type_cast< model::Model* >(
			const_cast< Object* >(buildParams)
		);
	}
	else
	{
		Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + meshAsset->getFileName());
		model = model::ModelCache::getInstance().getMutable(m_modelCachePath, filePath, meshAsset->getImportFilter());
	}

	if (!model)
	{
		log::error << L"Physics mesh pipeline failed; no model." << Endl;
		return false;
	}
	if (model->getPositions().empty() || model->getVertices().empty() || model->getPolygons().empty())
	{
		log::error << L"Physics mesh pipeline failed; no geometry." << Endl;
		return false;
	}

	// Cleanup model suitable for physics.
	model->clear(model::Model::CfColors | model::Model::CfNormals | model::Model::CfTexCoords | model::Model::CfJoints);
	model::CleanDuplicates(0.01f).apply(*model);

	model::Transform(
		scale(meshAsset->getScaleFactor(), meshAsset->getScaleFactor(), meshAsset->getScaleFactor())
	).apply(*model);

	if (meshAsset->getCenter())
	{
		const Aabb3 boundingBox = model->getBoundingBox();
		model::Transform(translate(-boundingBox.getCenter())).apply(*model);
	}

	if (meshAsset->getGrounded())
	{
		const Aabb3 boundingBox = model->getBoundingBox();
		model::Transform(translate(Vector4(0.0f, -boundingBox.mn.y(), 0.0f))).apply(*model);
	}

	// Shrink model by margin; need to calculate normals from positions only
	// as we don't want smooth groups or anything else mess with the normals.
	if (abs(meshAsset->m_margin) > FUZZY_EPSILON)
	{
		model::CalculateNormals().apply(*model);
		model::ScaleAlongNormal(-meshAsset->m_margin).apply(*model);
		model->clear(model::Model::CfNormals);
		model::CleanDuplicates(0.01f).apply(*model);
	}

	// Triangulate and ensure no degenerate polygons.
	model::Triangulate().apply(*model);
	model::CleanDegenerate().apply(*model);
	model::CleanDuplicates(0.01f).apply(*model);

	// Calculate bounding box; used for center of gravity estimation.
	const Aabb3 boundingBox = model->getBoundingBox();
	Vector4 centerOfGravity = boundingBox.getCenter().xyz0();

	// Create physics mesh.
	AlignedVector< Vector4 > positions = model->getPositions();
	for (auto& position : positions)
		position -= centerOfGravity;

	AlignedVector< Vector4 > normals;
	AlignedVector< Mesh::Triangle > meshShapeTriangles;
	AlignedVector< Mesh::Triangle > meshHullTriangles;
	AlignedVector< uint32_t > meshHullIndices;
	AlignedVector< Mesh::Material > meshMaterials;

	// Add default material first.
	meshMaterials.push_back({ 0.75f, 0.5f });

	// Add materials defined in asset, build map from
	// model material to resource materials.
	SmallMap< uint32_t, uint32_t > materialMap;
	for (auto it : meshAsset->getMaterials())
	{
		const auto& materials = model->getMaterials();
		auto it2 = std::find_if(materials.begin(), materials.end(), [&](const model::Material& m) -> bool {
			return it.first == m.getName();
		});
		if (it2 == materials.end())
		{
			log::warning << L"Material \"" << it.first << L"\" do not exist in source model." << Endl;
			continue;
		}
		const uint32_t index = std::distance(materials.begin(), it2);

		auto materialDef = pipelineBuilder->getObjectReadOnly< Material >(it.second);
		if (!materialDef)
			return false;
		
		materialMap[index] = (uint32_t)meshMaterials.size();
		meshMaterials.push_back({ materialDef->getFriction(), materialDef->getRestitution() });
	}

	// Convert source triangles into collision mesh resource.
	for (const auto& triangle : model->getPolygons())
	{
		T_ASSERT(triangle.getVertices().size() == 3);

		Mesh::Triangle shapeTriangle;
		for (int i = 0; i < 3; ++i)
			shapeTriangle.indices[i] = model->getVertex(triangle.getVertex(i)).getPosition();

		auto it = materialMap.find(triangle.getMaterial());
		if (it != materialMap.end())
			shapeTriangle.material = it->second;
		else
			shapeTriangle.material = 0;

		meshShapeTriangles.push_back(shapeTriangle);

		const Vector4 e1 = positions[shapeTriangle.indices[2]] - positions[shapeTriangle.indices[1]];
		const Vector4 e2 = positions[shapeTriangle.indices[0]] - positions[shapeTriangle.indices[1]];

		normals.push_back(cross(e2, e1).normalized());
	}

	if (meshAsset->m_calculateConvexHull)
	{
		log::info << L"Calculating convex hull..." << Endl;

		model::Model hull = *model;
		model::CalculateConvexHull().apply(hull);

		// Extract hull triangles.
		for (const auto& triangle : hull.getPolygons())
		{
			T_ASSERT(triangle.getVertices().size() == 3);

			Mesh::Triangle hullTriangle;
			for (int i = 0; i < 3; ++i)
				hullTriangle.indices[i] = hull.getVertex(triangle.getVertex(i)).getPosition();

			auto it = materialMap.find(triangle.getMaterial());
			if (it != materialMap.end())
				hullTriangle.material = it->second;
			else
				hullTriangle.material = 0;

			meshHullTriangles.push_back(hullTriangle);
		}

		// Extract hull indices.
		SmallSet< uint32_t > uniqueIndices;
		for (const auto& triangle : hull.getPolygons())
		{
			uniqueIndices.insert(hull.getVertex(triangle.getVertex(0)).getPosition());
			uniqueIndices.insert(hull.getVertex(triangle.getVertex(1)).getPosition());
			uniqueIndices.insert(hull.getVertex(triangle.getVertex(2)).getPosition());
		}
		for (const auto uniqueIndex : uniqueIndices)
			meshHullIndices.push_back(uniqueIndex);

		// Improve center of gravity by weighting in volumes of each hull face tetrahedron.
		Vector4 Voffset = Vector4::zero();
		Scalar Vtotal = 0.0_simd;
		for (AlignedVector< Mesh::Triangle >::const_iterator i = meshHullTriangles.begin(); i != meshHullTriangles.end(); ++i)
		{
			const Vector4 a = positions[i->indices[0]];
			const Vector4 b = positions[i->indices[1]];
			const Vector4 c = positions[i->indices[2]];

			const Scalar V = abs(dot3(a, cross(b, c))) / 6.0_simd;
			const Vector4 C = (a + b + c) / 4.0_simd;

			Voffset += C * V;
			Vtotal += V;
		}

		for (auto& position : positions)
			position -= Voffset / Vtotal;

		centerOfGravity += Voffset / Vtotal;
		log::debug << L"Hull volume " << Vtotal << L" unit^3." << Endl;
	}

	// Log statistics.
	log::debug << int32_t(positions.size()) << L" vertex(es)." << Endl;
	log::debug << int32_t(meshShapeTriangles.size()) << L" shape triangle(s)." << Endl;
	if (meshAsset->m_calculateConvexHull)
	{
		log::debug << int32_t(meshHullTriangles.size()) << L" hull triangle(s)." << Endl;
		log::debug << L"Offset " << centerOfGravity << L"." << Endl;
	}
	log::debug << meshAsset->m_margin << L" unit(s) margin." << Endl;

	Mesh mesh;
	mesh.setVertices(positions);
	mesh.setNormals(normals);
	mesh.setShapeTriangles(meshShapeTriangles);
	mesh.setHullTriangles(meshHullTriangles);
	mesh.setHullIndices(meshHullIndices);
	mesh.setMaterials(meshMaterials);
	mesh.setOffset(centerOfGravity);
	mesh.setMargin(meshAsset->m_margin);

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Physics mesh pipeline failed; unable create output instance." << Endl;
		return false;
	}

	instance->setObject(new MeshResource());

	Ref< IStream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Physics mesh pipeline failed; unable to write data." << Endl;
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
		log::error << L"Physics mesh pipeline failed; unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > MeshPipeline::buildProduct(
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
