/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Instance.h"
#include "Editor/DefaultPipeline.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.DefaultPipeline", 2, DefaultPipeline, IPipeline)

bool DefaultPipeline::create(const IPipelineSettings* settings)
{
	return true;
}

void DefaultPipeline::destroy()
{
}

TypeInfoSet DefaultPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< Object >();
}

bool DefaultPipeline::shouldCache() const
{
	return false;
}

uint32_t DefaultPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool DefaultPipeline::buildDependencies(
	IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< Reflection > reflection = Reflection::create(sourceAsset);
	if (!reflection)
		return false;

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< const RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (objectMember->get() != nullptr && objectMember->get() != sourceAsset)
			pipelineDepends->addDependency(objectMember->get());
	}

	return true;
}

bool DefaultPipeline::buildOutput(
	IPipelineBuilder* pipelineBuilder,
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
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance." << Endl;
		return false;
	}

	outputInstance->setObject(sourceAsset);

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > DefaultPipeline::buildProduct(
	IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

}
