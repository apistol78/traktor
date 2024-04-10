/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Editor/IPipelineDepends.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshComponentPipeline.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshComponentPipeline", 0, MeshComponentPipeline, editor::IPipeline)

bool MeshComponentPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void MeshComponentPipeline::destroy()
{
}

TypeInfoSet MeshComponentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< MeshComponentData >();
}

bool MeshComponentPipeline::shouldCache() const
{
	return false;
}

uint32_t MeshComponentPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool MeshComponentPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const MeshComponentData* meshComponentData = dynamic_type_cast< const MeshComponentData* >(sourceAsset))
		pipelineDepends->addDependency(meshComponentData->getMesh(), editor::PdfBuild | editor::PdfResource);
	return true;
}

bool MeshComponentPipeline::buildOutput(
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
	T_FATAL_ERROR;
	return false;
}

Ref< ISerializable > MeshComponentPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	return DeepClone(sourceAsset).create();
}

}
