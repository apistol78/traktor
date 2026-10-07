/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/MarkerEntityComponentPipeline.h"

#include "Core/Settings/PropertyBoolean.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Scene/Editor/MarkerEntityComponentData.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.MarkerEntityComponentPipeline", 0, MarkerEntityComponentPipeline, world::EntityPipeline)

bool MarkerEntityComponentPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	if (!world::EntityPipeline::create(settings, database))
		return false;

	m_targetEditor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false);
	return true;
}

TypeInfoSet MarkerEntityComponentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< MarkerEntityComponentData >();
}

bool MarkerEntityComponentPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const MarkerEntityComponentData* markerComponentData = mandatory_non_null_type_cast< const MarkerEntityComponentData* >(sourceAsset);

	// Texture is only used by the scene editor.
	if (m_targetEditor)
		pipelineDepends->addDependency(markerComponentData->getTexture(), editor::PdfBuild | editor::PdfResource);

	return true;
}

Ref< ISerializable > MarkerEntityComponentPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	return nullptr;
}

}
