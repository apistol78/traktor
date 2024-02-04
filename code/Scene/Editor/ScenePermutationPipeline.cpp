/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/ScenePermutationAsset.h"
#include "Scene/Editor/ScenePermutationPipeline.h"
#include "World/EntityData.h"
#include "World/WorldRenderSettings.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.ScenePermutationPipeline", 4, ScenePermutationPipeline, editor::IPipeline)

bool ScenePermutationPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void ScenePermutationPipeline::destroy()
{
}

TypeInfoSet ScenePermutationPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< ScenePermutationAsset >();
}

bool ScenePermutationPipeline::shouldCache() const
{
	return false;
}

uint32_t ScenePermutationPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool ScenePermutationPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const ScenePermutationAsset* scenePermutationAsset = mandatory_non_null_type_cast< const ScenePermutationAsset* >(sourceAsset);

	Ref< const SceneAsset > templateScene = pipelineDepends->getObjectReadOnly< SceneAsset >(scenePermutationAsset->m_scene);
	if (!templateScene)
	{
		log::error << L"Scene permutation pipeline failed; Unable to read scene template." << Endl;
		return false;
	}

	pipelineDepends->addDependency(scenePermutationAsset->m_scene, editor::PdfUse);
	return true;
}

bool ScenePermutationPipeline::buildOutput(
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
	Ref< const SceneAsset > scenePermutation = checked_type_cast< const SceneAsset*, true >(buildProduct(pipelineBuilder, sourceInstance, sourceAsset, buildParams));
	if (!scenePermutation)
	{
		log::error << L"Scene permutation pipeline failed; unable to generate scene permutation." << Endl;
		return false;
	}

	return pipelineBuilder->buildAdHocOutput(
		scenePermutation,
		outputPath,
		outputGuid
	);
}

Ref< ISerializable > ScenePermutationPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	const ScenePermutationAsset* scenePermutationAsset = mandatory_non_null_type_cast< const ScenePermutationAsset* >(sourceAsset);

	Ref< const SceneAsset > templateScene = pipelineBuilder->getObjectReadOnly< SceneAsset >(scenePermutationAsset->m_scene);
	T_ASSERT(templateScene);

	const std::list< std::wstring >& includeLayers = scenePermutationAsset->m_includeLayers;

	Ref< SceneAsset > scenePermutation = DeepClone(templateScene).create< SceneAsset >();
	T_ASSERT(scenePermutation);

	if (scenePermutationAsset->m_overrideWorldRenderSettings)
		scenePermutation->setWorldRenderSettings(scenePermutationAsset->m_overrideWorldRenderSettings);

	//for (auto layer : scenePermutation->getLayers())
	//{
	//	if (std::find(includeLayers.begin(), includeLayers.end(), layer->getName()) != includeLayers.end())
	//	{
	//		auto editorAttributes = layer->getComponent< world::EditorAttributesComponentData >();
	//		if (editorAttributes)
	//			editorAttributes->include = true;
	//	}
	//}

	return scenePermutation;
}

}
