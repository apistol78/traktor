/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Cloth/Cloth.h"
#include "Animation/Editor/Cloth/ClothAsset.h"
#include "Animation/Editor/Cloth/ClothPipeline.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/Operations/Triangulate.h"

namespace traktor::animation
{
	namespace
	{
	
AlignedVector< uint32_t > findAllVerticesSharingPosition(const model::Model* model, uint32_t positionId)
{
	AlignedVector< uint32_t > sharing;
	for (uint32_t i = 0; i < model->getVertexCount(); ++i)
	{
		if (model->getVertex(i).getPosition() == positionId)
			sharing.push_back(i);
	}
	return sharing;
}

AlignedVector< uint32_t > findAllPolygonsSharingVertex(const model::Model* model, uint32_t vertexId)
{
	AlignedVector< uint32_t > sharing;
	for (uint32_t i = 0; i < model->getPolygonCount(); ++i)
	{
		const model::Polygon& polygon = model->getPolygon(i);
		if (std::find(polygon.getVertices().begin(), polygon.getVertices().end(), vertexId) != polygon.getVertices().end())
			sharing.push_back(i);
	}
	return sharing;
}

uint32_t indexOfVertexInPolygon(const model::Polygon& polygon, uint32_t vertexId)
{
	for (uint32_t i = 0; i < polygon.getVertexCount(); ++i)
	{
		if (polygon.getVertex(i) == vertexId)
			return i;
	}
	return model::c_InvalidIndex;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.ClothPipeline", 20, ClothPipeline, editor::IPipeline)

bool ClothPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	m_modelCachePath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.ModelCache.Path");
	return true;
}

void ClothPipeline::destroy()
{
}

TypeInfoSet ClothPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< ClothAsset >();
}

uint32_t ClothPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool ClothPipeline::shouldCache() const
{
	return true;
}

bool ClothPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< const ClothAsset > clothAsset = mandatory_non_null_type_cast< const ClothAsset* >(sourceAsset);
	pipelineDepends->addDependency(m_assetPath, clothAsset->getFileName().getPathName());
	return true;
}

bool ClothPipeline::buildOutput(
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
	Ref< const ClothAsset > clothAsset = mandatory_non_null_type_cast< const ClothAsset* >(sourceAsset);

	// Read source model.
	const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + clothAsset->getFileName());
	Ref< model::Model > model = model::ModelCache::getInstance().getMutable(m_modelCachePath, filePath, L"");
	if (!model)
	{
		log::error << L"Unable to build cloth; no such file \"" << clothAsset->getFileName().getPathName() << L"\"." << Endl;
		return false;
	}

	Ref< Cloth > cloth = new Cloth();

	const uint32_t fixedJoint = model->findJointIndex(L"Fixed");

	cloth->m_nodes.reserve(model->getPositionCount());
	for (uint32_t i = 0; i < model->getPositionCount(); ++i)
	{
		const auto& position = model->getPosition(i);

		Cloth::Node& n = cloth->m_nodes.push_back();
		n.position = position;
		n.texCoord = Vector2::zero();
		n.invMass = 1.0f;
		n.east = -1;
		n.north = -1;

		const AlignedVector< uint32_t > sharingVertices = findAllVerticesSharingPosition(model, i);

		// Determine node mass.
		for (uint32_t sharingVertex : sharingVertices)
		{
			const model::Vertex& vertex = model->getVertex(sharingVertex);
			if (vertex.getPosition() == i)
			{
				if (vertex.getJointInfluence(fixedJoint) > 0.0f)
				{
					n.invMass = 0.0f;
					break;
				}
			}
		}

		if (sharingVertices.size() >= 1)
		{
			const AlignedVector< uint32_t > sharingPolygons = findAllPolygonsSharingVertex(model, sharingVertices.front());
			if (sharingPolygons.size() >= 1)
			{
				const model::Polygon& sharingPolygon = model->getPolygon(sharingPolygons.front());

				const uint32_t idx0 = indexOfVertexInPolygon(sharingPolygon, sharingVertices.front());
				const uint32_t idx1 = (idx0 + 1) % sharingPolygon.getVertexCount();
				const uint32_t idx2 = (idx0 + 2) % sharingPolygon.getVertexCount();

				n.east = model->getVertex(idx1).getPosition();
				n.north = model->getVertex(idx2).getPosition();
			}
		}
	}

	// Add explicit edges.
	for (const auto& polygon : model->getPolygons())
	{
		if (polygon.getVertexCount() == 2)
		{
			const uint32_t vxa = polygon.getVertex(0);
			const uint32_t vxb = polygon.getVertex(1);

			const Vector4 pa = model->getVertexPosition(vxa);
			const Vector4 pb = model->getVertexPosition(vxb);
			const Scalar ln = (pb - pa).length();

			Cloth::Edge& e = cloth->m_edges.push_back();
			e.indices[0] = model->getVertex(vxa).getPosition();
			e.indices[1] = model->getVertex(vxb).getPosition();
			e.length = ln;
		}
	}

	// Add edges from quads.
	for (const auto& polygon : model->getPolygons())
	{
		if (polygon.getVertexCount() == 4)
		{
			const int32_t edgeIndices[] =
			{
				0, 1,
				1, 2,
				2, 3,
				3, 0,
				0, 2,
				1, 3
			};

			for (int32_t i = 0; i < sizeof_array(edgeIndices); i += 2)
			{
				const uint32_t vxa = polygon.getVertex(edgeIndices[i]);
				const uint32_t vxb = polygon.getVertex(edgeIndices[i + 1]);

				const Vector4 pa = model->getVertexPosition(vxa);
				const Vector4 pb = model->getVertexPosition(vxb);
				const Scalar ln = (pb - pa).length();

				Cloth::Edge& e = cloth->m_edges.push_back();
				e.indices[0] = model->getVertex(vxa).getPosition();
				e.indices[1] = model->getVertex(vxb).getPosition();
				e.length = ln;
			}
		}
	}

	// Triangulate all quads.
	model::Triangulate().apply(*model);

	// Create triangle index buffer.
	for (const auto& polygon : model->getPolygons())
	{
		if (polygon.getVertexCount() == 3)
		{
			for (int32_t i = 0; i < 3; ++i)
			{
				cloth->m_triangles.push_back(
					model->getVertex(polygon.getVertex(i)).getPosition()
				);
			}
		}
	}

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!instance)
	{
		log::error << L"Unable to build cloth; unable to create output instance \"" << outputPath << L"\"." << Endl;
		return false;
	}

	instance->setObject(cloth);

	if (!instance->commit())
	{
		log::error << L"Unable to build cloth; unable to commit output instance \"" << outputPath << L"\"." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > ClothPipeline::buildProduct(
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
