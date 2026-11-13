/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/EntityStreamingHLODComponentPipeline.h"

#include "Scene/Editor/EntityStreamingHLODComponentData.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.EntityStreamingHLODComponentPipeline", 0, EntityStreamingHLODComponentPipeline, world::EntityPipeline)

TypeInfoSet EntityStreamingHLODComponentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< EntityStreamingHLODComponentData >();
}

bool EntityStreamingHLODComponentPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	return true;
}

Ref< ISerializable > EntityStreamingHLODComponentPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	return nullptr;
}

}
