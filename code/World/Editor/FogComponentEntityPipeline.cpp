/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "World/EntityData.h"
#include "World/IEntityComponentData.h"
#include "World/IEntityEventData.h"

#include "World/Entity/FogComponentData.h"
#include "World/Editor/FogComponentEntityPipeline.h"

namespace traktor::world
{
namespace
{

const Guid c_scatteringShaderTemplate(L"{FEDA90CE-25C6-BC4D-9767-EA4B45F4A043}");

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.FogComponentEntityPipeline", 0, FogComponentEntityPipeline, EntityPipeline)

bool FogComponentEntityPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	return true;
}

void FogComponentEntityPipeline::destroy()
{
}

TypeInfoSet FogComponentEntityPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< FogComponentData >();
}

bool FogComponentEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const FogComponentData* fogComponent = mandatory_non_null_type_cast< const FogComponentData* >(sourceAsset);
	pipelineDepends->addDependency(fogComponent->m_mediumShader, editor::PdfUse);
	pipelineDepends->addDependency(c_scatteringShaderTemplate, editor::PdfUse);
	return true;
}

Ref< ISerializable > FogComponentEntityPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	const FogComponentData* fogComponent = mandatory_non_null_type_cast< const FogComponentData* >(sourceAsset);

	// Create a complete scattering shader from the template, with the medium
	// fragment replaced by the one referenced by the component.
	Ref< render::ShaderGraph > scatteringShaderGraph = DeepClone(
		pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(c_scatteringShaderTemplate)
	).create< render::ShaderGraph >();
	if (!scatteringShaderGraph)
	{
		log::error << L"Fog component pipeline failed; unable to read scattering shader template." << Endl;
		return nullptr;
	}

	bool mediumReplaced = false;
	for (auto externalNode : scatteringShaderGraph->findNodesOf< render::External >())
	{
		if (trim(externalNode->getComment()) == L"Tag_Medium")
		{
			externalNode->setFragmentGuid(fogComponent->m_mediumShader);
			mediumReplaced = true;
		}
	}
	if (!mediumReplaced)
	{
		log::error << L"Fog component pipeline failed; no \"Tag_Medium\" external node in scattering shader template." << Endl;
		return nullptr;
	}

	// Resolve the graph fully before the ad-hoc build so the fragment contents,
	// not just their guids, are part of the ad-hoc source asset's hash. Left
	// unresolved, an edit to the medium fragment never changes the ad-hoc hash -
	// the fragment is already tracked by the outer scene build, so the ad-hoc
	// dependency scan filters it out and keeps finding the old shader in cache.
	scatteringShaderGraph = render::FragmentLinker([&](const Guid& fragmentGuid) {
		return pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(fragmentGuid);
	}).resolve(scatteringShaderGraph, true);
	if (!scatteringShaderGraph)
	{
		log::error << L"Fog component pipeline failed; unable to resolve scattering shader." << Endl;
		return nullptr;
	}

	const Guid outputGuid = fogComponent->m_mediumShader.permutation(FogComponentData::ms_generatedShaderSeed);

	if (!pipelineBuilder->buildAdHocOutput(
			scatteringShaderGraph,
			L"Generated/" + outputGuid.format(),
			outputGuid))
	{
		log::error << L"Fog component pipeline failed; unable to build scattering shader." << Endl;
		return nullptr;
	}

	// Keep the source medium guid in the product; FogComponentData::createComponent
	// derives the generated shader's guid with the same permutation, and the scene
	// editor creates components from unbuilt source data. Storing the permuted guid
	// here would make createComponent permute it a second time - permutation is an
	// XOR, so the built game data would resolve back to the raw fragment guid.
	return new FogComponentData(*fogComponent);
}

}
