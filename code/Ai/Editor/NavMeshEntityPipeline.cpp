/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ai/NavMeshEntityData.h"
#include "Ai/Editor/NavMeshEntityPipeline.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshEntityPipeline", 1, NavMeshEntityPipeline, world::EntityPipeline)

TypeInfoSet NavMeshEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< NavMeshEntityData >());
	return typeSet;
}

bool NavMeshEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const NavMeshEntityData* entityData = checked_type_cast< const NavMeshEntityData*, false >(sourceAsset);

	if (!world::EntityPipeline::buildDependencies(pipelineDepends, sourceInstance, sourceAsset, outputPath, outputGuid))
		return false;

	pipelineDepends->addDependency(entityData->get(), editor::PdfResource | editor::PdfBuild);
	return true;
}

	}
}
