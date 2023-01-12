/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMeshComponentData.h"
#include "Ai/Editor/NavMeshEntityPipeline.h"
#include "Editor/IPipelineDepends.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshEntityPipeline", 1, NavMeshEntityPipeline, world::EntityPipeline)

TypeInfoSet NavMeshEntityPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< NavMeshComponentData >();
}

bool NavMeshEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const NavMeshComponentData* entityData = checked_type_cast< const NavMeshComponentData*, false >(sourceAsset);

	if (!world::EntityPipeline::buildDependencies(pipelineDepends, sourceInstance, sourceAsset, outputPath, outputGuid))
		return false;

	pipelineDepends->addDependency(entityData->get(), editor::PdfResource | editor::PdfBuild);
	return true;
}

}
