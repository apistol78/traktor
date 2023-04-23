/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Shape/Editor/Prefab/PrefabComponentData.h"
#include "Shape/Editor/Prefab/PrefabComponentPipeline.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.PrefabComponentPipeline", 1, PrefabComponentPipeline, world::EntityPipeline)

bool PrefabComponentPipeline::create(const editor::IPipelineSettings* settings)
{
	m_editor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false);
	return true;
}

TypeInfoSet PrefabComponentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< PrefabComponentData >();
}

Ref< ISerializable > PrefabComponentPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	// Do not build prefab in editor since it's time consuming and not
	// strictly necessary for editing purposes.
	if (m_editor)
	{
		return world::EntityPipeline::buildProduct(
			pipelineBuilder,
			sourceInstance,
			sourceAsset,
			buildParams
		);
	}

	// Transform component into a plain group component.
	const PrefabComponentData* prefabComponent = mandatory_non_null_type_cast< const PrefabComponentData* >(sourceAsset);
	return world::EntityPipeline::buildProduct(
		pipelineBuilder,
		sourceInstance,
		new world::GroupComponentData(prefabComponent->getEntityData()),
		buildParams
	);
}

}
