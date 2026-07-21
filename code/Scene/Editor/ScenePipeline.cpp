/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineCommon.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Editor/Pipeline/PipelineProfiler.h"
#include "Scene/SceneResource.h"
#include "Scene/Editor/ISceneOperationData.h"
#include "Scene/Editor/ScenePipeline.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/Traverser.h"
#include "World/IWorldComponentData.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/GroupComponentData.h"

namespace traktor::scene
{
namespace
{

/*! Scene transform context backed by a pipeline common interface.
 *
 * Provides read only object/database access to operators running at pipeline
 * time. No live ground sampler is supplied; terrain aware operators construct
 * their own sampler from the source data (getSourceDatabase/getObjectReadOnly).
 */
class PipelineTransformContext : public ISceneOperator::TransformContext
{
public:
	explicit PipelineTransformContext(editor::IPipelineCommon* pipelineCommon)
	:	m_pipelineCommon(pipelineCommon)
	{
	}

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) const override final
	{
		return m_pipelineCommon->getObjectReadOnly(instanceGuid);
	}

	virtual db::Database* getSourceDatabase() const override final
	{
		return m_pipelineCommon->getSourceDatabase();
	}

private:
	editor::IPipelineCommon* m_pipelineCommon;
};

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.ScenePipeline", 20, ScenePipeline, editor::IPipeline)

bool ScenePipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	m_targetEditor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false);
	m_suppressShadows = settings->getPropertyIncludeHash< bool >(L"ScenePipeline.SuppressShadows", false);
	m_suppressDepthPass = settings->getPropertyIncludeHash< bool >(L"ScenePipeline.SuppressDepthPass", false);
	m_suppressImageProcess = settings->getPropertyIncludeHash< bool >(L"ScenePipeline.SuppressImageProcess", false);
	m_shadowMapSizeDenom = settings->getPropertyIncludeHash< int32_t >(L"ScenePipeline.ShadowMapSizeDenom", 1);
	m_shadowMapMaxSlices = settings->getPropertyIncludeHash< int32_t >(L"ScenePipeline.ShadowMapMaxSlices", 0);

	// Instantiate scene pipeline operators.
	for (auto operatorType : type_of< ISceneOperator >().findAllOf(false))
	{
		Ref< ISceneOperator > spo = dynamic_type_cast< ISceneOperator* >(operatorType->createInstance());
		if (!spo)
		{
			log::error << L"Failed to create scene pipeline; unable to instantiate operator \"" << operatorType->getName() << L"\"." << Endl;
			return false;
		}
		if (!spo->create(settings))
		{
			log::error << L"Failed to create scene pipeline; unable to create operator \"" << operatorType->getName() << L"\"." << Endl;
			return false;
		}
		m_operators.push_back(spo);
	}

	return true;
}

void ScenePipeline::destroy()
{
	for (auto op : m_operators)
		op->destroy();
	m_operators.clear();
}

TypeInfoSet ScenePipeline::getAssetTypes() const
{
	return makeTypeInfoSet< SceneAsset >();
}

bool ScenePipeline::shouldCache() const
{
	return true;
}

uint32_t ScenePipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool ScenePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const SceneAsset* sceneAsset = mandatory_non_null_type_cast< const SceneAsset* >(sourceAsset);

	// Transform, or filter, scene asset through operators.
	PipelineTransformContext context(pipelineDepends);
	Ref< SceneAsset > mutableSceneAsset = DeepClone(sceneAsset).create< SceneAsset >();
	for (const auto operationData : sceneAsset->getOperationData())
	{
		const ISceneOperator* spo = findOperator(type_of(operationData));
		if (!spo)
		{
			log::error << L"Scene pipeline failed; no operator found supporting data type " << type_name(operationData) << L"." << Endl;
			return false;
		}

		spo->addDependencies(pipelineDepends, operationData);

		if (!spo->transform(context, operationData, mutableSceneAsset))
		{
			log::error << L"Scene pipeline failed; operator transform failed." << Endl;
			return false;
		}
	}

	// Add dependencies from scene asset.
	for (const auto& worldComponent : mutableSceneAsset->getWorldComponents())
		pipelineDepends->addDependency(worldComponent);
	for (const auto& layer : mutableSceneAsset->getLayers())
		pipelineDepends->addDependency(layer);

	// In case we're building for the editor we also need to add dependencies to the unmodified scene.
	if (m_targetEditor)
	{
		for (const auto& layer : sceneAsset->getLayers())
			pipelineDepends->addDependency(layer);
	}

	const world::WorldRenderSettings* wrs = mutableSceneAsset->getWorldRenderSettings();
	if (wrs)
	{
		if (!m_suppressImageProcess)
		{
			for (int32_t i = 0; i < sizeof_array(wrs->imageProcess); ++i)
				pipelineDepends->addDependency(wrs->imageProcess[i], editor::PdfBuild);
		}

		pipelineDepends->addDependency(wrs->colorGrading, editor::PdfBuild | editor::PdfResource);
	}

	return true;
}

bool ScenePipeline::buildOutput(
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
	// Create clone of asset which operators can modify.
	Ref< SceneAsset > sceneAsset = DeepClone(sourceAsset).create< SceneAsset >();
	T_FATAL_ASSERT (sceneAsset != nullptr);

	const bool rebuild = (bool)((reason & editor::PbrForced) != 0);

	// Apply geometric scene transforms first.
	PipelineTransformContext context(pipelineBuilder);
	if (!applyTransforms(sceneAsset, context))
		return false;

	// Execute build operations on scene.
	log::info << L"Executing scene operations..." << Endl;
	log::info << IncreaseIndent;
	for (const auto operationData : sceneAsset->getOperationData())
	{
		const ISceneOperator* sceneOperator = findOperator(type_of(operationData));
		if (!sceneOperator)
			return false;

		pipelineBuilder->getProfiler()->begin(type_of(sceneOperator));
		const bool result = sceneOperator->build(pipelineBuilder, operationData, sourceInstance, sceneAsset, rebuild);
		pipelineBuilder->getProfiler()->end();

		if (!result)
			return false;
	}
	log::info << DecreaseIndent;

	// Build each layer of entity data; merge into a single output group.
	Ref< world::GroupComponentData > groupComponentData = new world::GroupComponentData();
	for (const auto& layer : sceneAsset->getLayers())
	{
		if (!layer)
			continue;
		if (!layer->getState().visible)
			continue;

		log::info << L"Building layer \"" << layer->getName() << L"\"..." << Endl;
		log::info << IncreaseIndent;

		auto layerGroupData = layer->getComponent< world::GroupComponentData >();
		if (layerGroupData != nullptr)
		{
			for (const auto& assetEntityData : layerGroupData->getEntityData())
			{
				Ref< world::EntityData > outputEntityData = checked_type_cast< world::EntityData*, true >(
					pipelineBuilder->buildProduct(sourceInstance, assetEntityData)
				);
				if (outputEntityData)
				{
					// Move dynamic state from layer to child.
					outputEntityData->setState(layer->getState(), world::EntityState::Dynamic);
					groupComponentData->addEntityData(outputEntityData);
				}
			}
		}

		log::info << DecreaseIndent;
	}

	Ref< world::EntityData > groupEntityData = new world::EntityData();
	groupEntityData->setComponent(groupComponentData);

	// Create output scene resource.
	Ref< SceneResource > sceneResource = new SceneResource();
	sceneResource->setWorldRenderSettings(sceneAsset->getWorldRenderSettings());
	sceneResource->setWorldComponents(sceneAsset->getWorldComponents());
	sceneResource->setEntityData(groupEntityData);

	for (uint32_t i = 0; i < (int32_t)world::Quality::Last; ++i)
	{
		if (m_suppressImageProcess)
			sceneResource->getWorldRenderSettings()->imageProcess[i] = resource::Id< render::ImageGraph >();
	}

	for (uint32_t i = 0; i < (int32_t)world::Quality::Last; ++i)
	{
		world::WorldRenderSettings::ShadowSettings& shadowSetting = sceneResource->getWorldRenderSettings()->shadowSettings[i];
		if (!m_suppressShadows)
		{
			shadowSetting.resolution /= m_shadowMapSizeDenom;
			if (m_shadowMapMaxSlices > 0)
				shadowSetting.cascadingSlices = std::min(shadowSetting.cascadingSlices, m_shadowMapMaxSlices);
		}
	}

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance \"" << outputPath << L"\"." << Endl;
		return false;
	}

	outputInstance->setObject(sceneResource);

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance \"" << outputPath << L"\"." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > ScenePipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	Ref< SceneAsset > sceneAsset = DeepClone(sourceAsset).create< SceneAsset >();
	if (!sceneAsset)
		return nullptr;

	PipelineTransformContext context(pipelineBuilder);
	if (!applyTransforms(sceneAsset, context))
		return nullptr;

	return sceneAsset;
}

const ISceneOperator* ScenePipeline::findOperator(const TypeInfo& operationType) const
{
	for (const auto sop : m_operators)
	{
		for (const auto ts : sop->getOperatorTypes())
		{
			if (is_type_a(*ts, operationType))
				return sop;
		}
	}
	return nullptr;
}

bool ScenePipeline::applyTransforms(SceneAsset* inoutSceneAsset, const ISceneOperator::TransformContext& context) const
{
	for (const auto op : inoutSceneAsset->getOperationData())
	{
		const ISceneOperator* spo = findOperator(type_of(op));
		if (!spo)
		{
			log::error << L"Scene pipeline failed; no operator found supporting data type " << type_name(op) << L"." << Endl;
			return false;
		}

		if (!spo->isGeometricTransform())
			continue;

		if (!spo->transform(context, op, inoutSceneAsset))
		{
			log::error << L"Scene pipeline failed; operator transform failed." << Endl;
			return false;
		}
	}
	return true;
}

}
