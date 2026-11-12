/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Math/Aabb3.h"
#include "Editor/DefaultPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct rcConfig;

namespace traktor::world
{

class IEntityReplicator;

}

namespace traktor::ai
{

class NavMeshAsset;
class NavMeshChunkAsset;

/*! Navigation mesh pipeline.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshPipeline : public editor::DefaultPipeline
{
	T_RTTI_CLASS;

public:
	//! Grid of chunks of a chunked navigation mesh.
	struct ChunkGrid
	{
		Aabb3 extent;	//!< Terrains, clipped by navigation volume.
		Vector4 origin = Vector4::origo();	//!< Corner of chunk 0, 0.
		float chunkWidth = 0.0f;
		int32_t chunkTiles = 0;
		int32_t firstX = 0;
		int32_t firstZ = 0;
		int32_t lastX = -1;
		int32_t lastZ = -1;
	};

	virtual bool create(const editor::IPipelineSettings* settings, db::Database* database) override final;

	virtual TypeInfoSet getAssetTypes() const override final;

	virtual bool shouldCache() const override final;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const override final;

	virtual bool buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const editor::PipelineDependencySet* dependencySet,
		const editor::PipelineDependency* dependency,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams,
		uint32_t reason
	) const override final;

	/*! Calculate grid of chunks of navigation mesh; empty if not chunked.
	 *
	 * \param pipelineBuilder Pipeline builder.
	 * \param asset Navigation mesh asset.
	 * \param outGrid Grid of chunks.
	 * eturn True if successful.
	 */
	static bool calculateChunkGrid(editor::IPipelineBuilder* pipelineBuilder, const NavMeshAsset* asset, ChunkGrid& outGrid);

	/*! Get id of chunk resource.
	 *
	 * \param navMeshId Id of navigation mesh.
	 * \param chunkX Chunk x coordinate.
	 * \param chunkZ Chunk z coordinate.
	 * eturn Id of chunk.
	 */
	static Guid getChunkId(const Guid& navMeshId, int32_t chunkX, int32_t chunkZ);

private:
	static Ref< const ISerializable > buildSourceScene(editor::IPipelineBuilder* pipelineBuilder, const NavMeshAsset* asset);

	static bool calculateChunkGrid(editor::IPipelineBuilder* pipelineBuilder, const NavMeshAsset* asset, const ISerializable* sourceData, ChunkGrid& outGrid);

	bool buildSingle(editor::IPipelineBuilder* pipelineBuilder, const db::Instance* sourceInstance, const NavMeshAsset* asset, const std::wstring& outputPath, const Guid& outputGuid) const;

	bool buildChunked(editor::IPipelineBuilder* pipelineBuilder, const db::Instance* sourceInstance, const NavMeshAsset* asset, const std::wstring& outputPath, const Guid& outputGuid) const;

	bool buildChunk(editor::IPipelineBuilder* pipelineBuilder, const NavMeshChunkAsset* chunkAsset, const std::wstring& outputPath, const Guid& outputGuid) const;

	static void setupConfig(const NavMeshAsset* asset, rcConfig& outCfg);

	std::wstring m_assetPath;
	bool m_editor = false;
	bool m_build = true;
	SmallMap< const TypeInfo*, Ref< const world::IEntityReplicator > > m_entityReplicators;
};

}
