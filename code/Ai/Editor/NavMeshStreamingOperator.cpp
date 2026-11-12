/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/Editor/NavMeshStreamingOperator.h"

#include "Ai/Editor/NavMeshAsset.h"
#include "Ai/Editor/NavMeshPipeline.h"
#include "Ai/Editor/NavMeshStreamingOperationData.h"
#include "Ai/NavMeshChunk.h"
#include "Ai/NavMeshChunkComponentData.h"
#include "Ai/NavMeshComponentData.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineSettings.h"
#include "Scene/Editor/SceneAsset.h"
#include "World/Entity/EntityStreamingComponentData.h"
#include "World/EntityData.h"

#include <algorithm>

namespace traktor::ai
{
namespace
{

const Guid c_guidChunkEntitySeed(L"{9C4A2E71-5B3D-4F08-B6E1-2D7F80A41C59}");

//! Id of entity of chunk.
Guid chunkEntityId(const Guid& chunkId)
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = c_guidChunkEntitySeed[i] ^ chunkId[i];
	return Guid(d);
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshStreamingOperator", 0, NavMeshStreamingOperator, scene::ISceneOperator)

bool NavMeshStreamingOperator::create(const editor::IPipelineSettings* settings)
{
	m_targetEditor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false);
	return true;
}

void NavMeshStreamingOperator::destroy()
{
}

TypeInfoSet NavMeshStreamingOperator::getOperatorTypes() const
{
	return makeTypeInfoSet< NavMeshStreamingOperationData >();
}

void NavMeshStreamingOperator::addDependencies(editor::IPipelineDepends* pipelineDepends, const ISerializable* operatorData) const
{
}

bool NavMeshStreamingOperator::transform(const scene::ISceneOperator::TransformContext& context, const ISerializable* operatorData, scene::SceneAsset* inoutSceneAsset) const
{
	return true;
}

bool NavMeshStreamingOperator::build(editor::IPipelineBuilder* pipelineBuilder, const ISerializable* operatorData, const db::Instance* sourceInstance, scene::SceneAsset* inoutSceneAsset, bool rebuild) const
{
	const auto data = mandatory_non_null_type_cast< const NavMeshStreamingOperationData* >(operatorData);

	// Editor doesn't stream entities, chunks are added around camera by editor instead.
	if (m_targetEditor)
		return true;

	const NavMeshComponentData* navMeshComponentData = inoutSceneAsset->getWorldComponent< NavMeshComponentData >();
	if (!navMeshComponentData)
		return true;

	Ref< const NavMeshAsset > navMeshAsset = pipelineBuilder->getObjectReadOnly< NavMeshAsset >(navMeshComponentData->get());
	if (!navMeshAsset)
	{
		log::error << L"NavMesh streaming operator failed; unable to read navigation mesh asset." << Endl;
		return false;
	}

	NavMeshPipeline::ChunkGrid grid;
	if (!NavMeshPipeline::calculateChunkGrid(pipelineBuilder, navMeshAsset, grid))
		return false;
	if (grid.lastX < grid.firstX || grid.lastZ < grid.firstZ)
		return true;

	world::EntityStreamingComponentData* entityStreaming = inoutSceneAsset->getWorldComponent< world::EntityStreamingComponentData >();
	if (!entityStreaming)
	{
		log::error << L"NavMesh streaming operator failed; scene has no entity streaming." << Endl;
		return false;
	}

	// Cells are grown so chunks are loaded within load distance.
	const float margin = std::max(data->getLoadDistance() - entityStreaming->getLoadDistance(), 0.0f);
	const Vector4 grow(margin, margin, margin, 0.0f);

	AlignedVector< world::EntityStreamingComponentData::Cell > cells = entityStreaming->getCells();
	for (int32_t cz = grid.firstZ; cz <= grid.lastZ; ++cz)
	{
		for (int32_t cx = grid.firstX; cx <= grid.lastX; ++cx)
		{
			const Guid chunkId = NavMeshPipeline::getChunkId(navMeshComponentData->get(), cx, cz);

			Ref< world::EntityData > chunkEntity = new world::EntityData();
			chunkEntity->setId(chunkEntityId(chunkId));
			chunkEntity->setName(L"NavMesh chunk " + toString(cx) + L"_" + toString(cz));
			chunkEntity->setComponent(new NavMeshChunkComponentData(resource::Id< NavMeshChunk >(chunkId)));

			const float x0 = grid.origin.x() + cx * grid.chunkWidth;
			const float z0 = grid.origin.z() + cz * grid.chunkWidth;
			const Aabb3 boundingBox(
				Vector4(x0, grid.extent.mn.y(), z0, 1.0f),
				Vector4(x0 + grid.chunkWidth, grid.extent.mx.y(), z0 + grid.chunkWidth, 1.0f));

			auto& cell = cells.push_back();
			cell.boundingBox = Aabb3(boundingBox.mn - grow, boundingBox.mx + grow);
			cell.entityData.push_back(chunkEntity);
		}
	}
	entityStreaming->setCells(cells);

	log::info << L"NavMesh streaming; " << (grid.lastX - grid.firstX + 1) * (grid.lastZ - grid.firstZ + 1) << L" chunk entities." << Endl;
	return true;
}

}
